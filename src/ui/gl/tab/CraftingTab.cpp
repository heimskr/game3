#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "threading/ThreadContext.h"
#include "ui/gl/tab/CraftingTab.h"
#include "ui/gl/widget/ProgressBarWidget.h"
#include "ui/gl/widget/TextInputWidget.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace {
	constexpr float scale = Game3::SCALE;

	auto & getTextInput() {
		static auto input = std::make_shared<Game3::TextInputWidget>(scale);
		return input;
	}
}

namespace Game3 {
	void CraftingTab::render(UIContext &ui, const RendererContext &renderers) {

		// auto bar = std::make_shared<ProgressBarWidget>(scale * 10, scale, Color(1, 0, 0, 1), 0.5);
		// bar->render(ui, renderers, 0, 0, scale * 100, scale * 10);

		getTextInput()->render(ui, renderers, 0, 0, scale * 100, scale * 10);
	}

	void CraftingTab::renderIcon(const RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/crafting.png"));
	}

	void CraftingTab::click(int button, int x, int y) {
		auto &input = getTextInput();
		if (input->getLastRectangle().contains(x, y))
			input->click(ui, button, x, y);
	}
}
