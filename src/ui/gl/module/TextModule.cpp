#include "graphics/RendererContext.h"
#include "graphics/TextRenderer.h"
#include "ui/gl/module/TextModule.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "util/Defer.h"

namespace Game3 {
	TextModule::TextModule(std::shared_ptr<ClientGame> game, const std::any &argument):
		TextModule(std::move(game), std::any_cast<std::string>(argument)) {}

	TextModule::TextModule(std::shared_ptr<ClientGame>, std::string text):
		text(std::move(text)) {}

	void TextModule::render(UIContext &ui, RendererContext &renderers, float x, float y, float width, float height) {
		auto saver = renderers.getSaver();
		ui.scissorStack.pushAbsolute(Rectangle(x, y, width, height));
		renderers.updateSize(width, height);
		Defer pop([&] { ui.scissorStack.pop(); });

		renderers.text.drawOnScreen(text, TextRenderOptions{
			.y = SCALE * 4 + 16,
			.scaleX = SCALE / 20,
			.scaleY = SCALE / 20,
			.wrapWidth = width - SCALE * 4,
			.color{0, 0, 0, 1},
			.shadow{0, 0, 0, 0},
		});
	}
}
