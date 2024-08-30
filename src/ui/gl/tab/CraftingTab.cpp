#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "threading/ThreadContext.h"
#include "ui/gl/tab/CraftingTab.h"
#include "ui/gl/widget/ButtonWidget.h"
#include "ui/gl/widget/ProgressBarWidget.h"
#include "ui/gl/widget/TextInputWidget.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "util/Util.h"

namespace {
	constexpr float scale = Game3::UI_SCALE;

	auto & getProgressBar(Game3::UIContext &) {
		static std::shared_ptr<Game3::ProgressBarWidget> bar = nullptr;

		if (!bar) {
			bar = std::make_shared<Game3::ProgressBarWidget>(scale, scale * 10, Game3::Color(1, 0, 0, 1), 0.5);
		}

		return bar;
	}

	auto & getTextInput(Game3::UIContext &ui) {
		static std::shared_ptr<Game3::TextInputWidget> input = nullptr;

		if (!input) {
			input = std::make_shared<Game3::TextInputWidget>(scale);
			input->setText(ui, "Hello from the crafting tab! This is some example text.");
			input->onSubmit = [&ui](Game3::TextInputWidget &input) {
				Glib::ustring text = input.clear();

				if (text.empty())
					return;

				float divisor = 1;
				if (text.substr(text.length() - 1, 1) == "%") {
					text.erase(text.length() - 1, 1);
					divisor = 100;
				}

				float number{};
				try {
					number = Game3::parseNumber<float>(text.raw()) / divisor;
				} catch (...) {
					input.setText(ui, "Invalid number.");
					return;
				}

				getProgressBar(ui)->setProgress(number);
			};
		}

		return input;
	}

	auto & getButton(Game3::UIContext &ui) {
		static std::shared_ptr<Game3::ButtonWidget> button = nullptr;

		if (!button) {
			button = std::make_shared<Game3::ButtonWidget>(scale, scale * 10);
			button->setText("Randomize");
			button->setOnClick([&, i = 0](auto &) mutable {
				auto &input = getTextInput(ui);
				Game3::INFO("Clicked {} time(s). Text = \"{}\"", ++i, input->getText().raw());
				const float progress = Game3::threadContext.random(0.f, 1.f);
				getProgressBar(ui)->setProgress(progress);
				input->setText(ui, std::format("{:.1f}%", progress * 100));
			});
		}

		return button;
	}
}

namespace Game3 {
	void CraftingTab::render(const RendererContext &renderers) {
		float y = 0;

		const float width = ui.scissorStack.getTop().rectangle.width;

		getTextInput(ui)->render(ui, renderers, 0, y, width, scale * TEXT_INPUT_HEIGHT_FACTOR);

		y += scale * (TEXT_INPUT_HEIGHT_FACTOR + 2);

		getProgressBar(ui)->render(ui, renderers, width / 4, y, width / 2, scale * 10);

		y += scale * 12;

		getButton(ui)->render(ui, renderers, scale * 2, y, -1, -1);
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
