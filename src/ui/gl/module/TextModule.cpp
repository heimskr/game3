#include "game/ClientGame.h"
#include "graphics/RendererContext.h"
#include "graphics/TextRenderer.h"
#include "ui/gl/module/TextModule.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/Canvas.h"
#include "util/Defer.h"

namespace Game3 {
	TextModule::TextModule(std::shared_ptr<ClientGame> game, const std::any &argument):
		TextModule(std::move(game), std::any_cast<std::string>(argument)) {}

	TextModule::TextModule(std::shared_ptr<ClientGame> game, std::string text):
		Module(UI_SCALE), weakGame(game) {
			setText(game->getUIContext(), std::move(text));
		}

	void TextModule::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		if (lastRectangle.width != width)
			wrapped.reset();

		Widget::render(ui, renderers, x, y, width, height);
		const float padding = getPadding();

		tryWrap();
		const UString &string = wrapped? wrapped.value() : text;

		renderers.text.drawOnScreen(string, TextRenderOptions{
			.x = x,
			.y = y + padding,
			.scaleX = getTextScale(),
			.scaleY = getTextScale(),
			.wrapWidth = wrapped? 0 : getWrapWidth(width),
			.color{0, 0, 0, 1},
			.alignTop = true,
			.shadow{0, 0, 0, 0},
			.heightOut = &lastTextHeight,
		});

		lastTextHeight += padding;
	}

	std::pair<float, float> TextModule::calculateSize(const RendererContext &renderers, float available_width, float available_height) {
		if (lastTextHeight > 0 && available_width == lastRectangle.width)
			return {available_width, lastTextHeight};

		tryWrap();

		if (!wrapped)
			return {available_width, available_height};

		return {available_width, renderers.text.textHeight(wrapped.value(), getTextScale(), available_width)};
	}

	void TextModule::setText(UIContext &ui, UString new_text) {
		if (text == new_text)
			return;

		text = std::move(new_text);
		wrapped.reset();
		tryWrap(ui.canvas.textRenderer);
	}

	float TextModule::getTextScale() const {
		return scale / 16;
	}

	float TextModule::getPadding() const {
		return scale * 2;
	}

	float TextModule::getWrapWidth(float width) const {
		return width - scale * 2;
	}

	void TextModule::tryWrap(const TextRenderer &texter) {
		if (!wrapped) {
			if (auto width = lastRectangle.width; width > 0) {
				wrapped = text.wrap(texter, getWrapWidth(width), getTextScale());
			}
		}
	}

	void TextModule::tryWrap() {
		tryWrap(getGame().canvas.textRenderer);
	}

	ClientGame & TextModule::getGame() const {
		if (auto game = weakGame.lock())
			return *game;
		throw std::runtime_error("Couldn't lock TextModule's game");
	}
}
