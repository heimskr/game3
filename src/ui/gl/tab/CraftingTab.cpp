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

	auto & getTextInput(Game3::UIContext &ui) {
		static std::shared_ptr<Game3::TextInputWidget> input = nullptr;

		if (!input) {
			input = std::make_shared<Game3::TextInputWidget>(scale);
			input->setText(ui, "Hello from the crafting tab! This is some example text.");
			input->onSubmit = [](Game3::TextInputWidget &input) {
				input.clear();
			};
		}

		return input;
	}
}

namespace Game3 {
	void CraftingTab::render(const RendererContext &renderers) {
		float y = 0;
		auto bar = std::make_shared<ProgressBarWidget>(scale * 10, scale, Color(1, 0, 0, 1), 0.5);
		bar->render(ui, renderers, 0, y, scale * 100, scale * 10);

		y += scale * 15;

		getTextInput(ui)->render(ui, renderers, 0, y, scale * 150, scale * TEXT_INPUT_HEIGHT_FACTOR);
	}

	void CraftingTab::renderIcon(const RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/crafting.png"));
	}

	void CraftingTab::click(int button, int x, int y) {
		auto &input = getTextInput(ui);
		if (input->getLastRectangle().contains(x, y))
			input->click(ui, button, x, y);
	}
}
