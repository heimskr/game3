#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "ui/gl/tab/CraftingTab.h"
#include "ui/gl/widget/ButtonWidget.h"
#include "ui/gl/widget/ProgressBarWidget.h"
#include "ui/gl/widget/TextInputWidget.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

#include "threading/ThreadContext.h"

namespace {
	constexpr float scale = Game3::UI_SCALE;

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

	auto & getProgressBar(Game3::UIContext &) {
		static std::shared_ptr<Game3::ProgressBarWidget> bar = nullptr;

		if (!bar) {
			bar = std::make_shared<Game3::ProgressBarWidget>(scale, scale * 10, Game3::Color(1, 0, 0, 1), 0.5);
		}

		return bar;
	}

	auto & getButton(Game3::UIContext &) {
		static std::shared_ptr<Game3::ButtonWidget> button = nullptr;

		if (!button) {
			button = std::make_shared<Game3::ButtonWidget>(scale, scale * 10);
			button->setText("Button");
			button->setOnClick([&](auto &) {
				Game3::INFO("Clicked");
			});
		}

		return button;
	}
}

namespace Game3 {
	void CraftingTab::render(const RendererContext &renderers) {
		float y = 0;

		getTextInput(ui)->render(ui, renderers, 0, y, scale * 150, scale * TEXT_INPUT_HEIGHT_FACTOR);

		y += scale * (TEXT_INPUT_HEIGHT_FACTOR + 4);

		getProgressBar(ui)->render(ui, renderers, 0, y, scale * 100, scale * 10);

		y += scale * 15;

		// for (int sc : {8, 10, 12, 16, 20, 40}) {
		// 	auto button = std::make_shared<ButtonWidget>(scale, scale * sc);
		// 	button->setText("Button");
		// 	button->render(ui, renderers, 0, y, -1, -1);
		// 	y += scale * (sc + 5);
		// }

		getButton(ui)->render(ui, renderers, 0, y, -1, -1);
	}

	void CraftingTab::renderIcon(const RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/crafting.png"));
	}

	void CraftingTab::click(int mouse_button, int x, int y) {
		auto &input = getTextInput(ui);
		if (input->getLastRectangle().contains(x, y))
			input->click(ui, mouse_button, x, y);

		auto &button = getButton(ui);
		if (button->getLastRectangle().contains(x, y))
			button->click(ui, mouse_button, x, y);
	}

	void CraftingTab::dragStart(int x, int y) {
		auto &input = getTextInput(ui);
		if (input->getLastRectangle().contains(x, y))
			input->dragStart(ui, x, y);

		auto &button = getButton(ui);
		if (button->getLastRectangle().contains(x, y))
			button->dragStart(ui, x, y);
	}

	void CraftingTab::dragEnd(int x, int y) {
		auto &input = getTextInput(ui);
		if (input->getLastRectangle().contains(x, y))
			input->dragEnd(ui, x, y);

		auto &button = getButton(ui);
		if (button->getLastRectangle().contains(x, y))
			button->dragEnd(ui, x, y);
	}
}
