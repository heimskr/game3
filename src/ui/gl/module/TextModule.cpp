#include "game/ClientGame.h"
#include "graphics/RendererContext.h"
#include "graphics/TextRenderer.h"
#include "ui/gl/module/TextModule.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/Window.h"
#include "util/Defer.h"

namespace Game3 {
	TextModule::TextModule(UIContext &ui, float selfScale, std::shared_ptr<ClientGame> game, const std::any &argument):
		TextModule(ui, selfScale, std::move(game), std::any_cast<std::string>(argument)) {}

	TextModule::TextModule(UIContext &ui, float selfScale, std::shared_ptr<ClientGame> game, std::string text):
		Module(ui, selfScale, game) {
			setText(game->getUIContext(), std::move(text));
		}

	void TextModule::render(const RendererContext &renderers, float x, float y, float width, float height) {
		if (lastRectangle.width != width) {
			wrapped.reset();
		}

		Widget::render(renderers, x, y, width, height);
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

	SizeRequestMode TextModule::getRequestMode() const {
		return SizeRequestMode::HeightForWidth;
	}

	void TextModule::measure(const RendererContext &renderers, Orientation orientation, float for_width, float, float &minimum, float &natural) {
		if (orientation == Orientation::Horizontal) {
			minimum = 0;
			natural = for_width;
			return;
		}

		// Add a little bit to account for descenders.
		const float addend = 2 * selfScale;

		if (lastTextHeight > 0 && for_width == lastRectangle.width) {
			minimum = natural = lastTextHeight + addend;
			return;
		}

		tryWrap();

		if (!wrapped) {
			minimum = natural = 0;
			return;
		}

		minimum = natural = renderers.text.textHeight(wrapped.value(), getTextScale(), for_width) + addend;
	}

	void TextModule::setText(UIContext &ui, UString new_text) {
		if (text == new_text)
			return;

		text = std::move(new_text);
		wrapped.reset();
		tryWrap(ui.window.textRenderer);
	}

	float TextModule::getTextScale() const {
		return selfScale / 16;
	}

	float TextModule::getPadding() const {
		return selfScale * 2;
	}

	float TextModule::getWrapWidth(float width) const {
		return width - selfScale * 2;
	}

	void TextModule::tryWrap(const TextRenderer &texter) {
		if (!wrapped) {
			if (auto width = lastRectangle.width; width > 0) {
				wrapped = text.wrap(texter, getWrapWidth(width), getTextScale());
			}
		}
	}

	void TextModule::tryWrap() {
		tryWrap(getGame()->getWindow()->textRenderer);
	}
}
