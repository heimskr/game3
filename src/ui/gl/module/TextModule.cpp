#include "graphics/RendererContext.h"
#include "graphics/TextRenderer.h"
#include "ui/gl/module/TextModule.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "util/Defer.h"

namespace {
	constexpr float TEXT_SCALE = Game3::SCALE / 16;
}

namespace Game3 {
	TextModule::TextModule(std::shared_ptr<ClientGame> game, const std::any &argument):
		TextModule(std::move(game), std::any_cast<std::string>(argument)) {}

	TextModule::TextModule(std::shared_ptr<ClientGame>, std::string text):
		text(std::move(text)) {}

	void TextModule::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		Widget::render(ui, renderers, x, y, width, height);

		renderers.text.drawOnScreen(text, TextRenderOptions{
			.x = x,
			.y = y,
			.scaleX = TEXT_SCALE,
			.scaleY = TEXT_SCALE,
			.wrapWidth = width - SCALE * 4,
			.color{0, 0, 0, 1},
			.alignTop = true,
			.shadow{0, 0, 0, 0},
			.heightOut = &lastTextHeight,
		});
	}

	float TextModule::calculateHeight(const RendererContext &renderers, float available_width, float) {
		if (lastTextHeight > 0 && available_width == lastWidth)
			return lastTextHeight;

		return renderers.text.textHeight(text, TEXT_SCALE, available_width);
	}
}
