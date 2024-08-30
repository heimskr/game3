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
	using namespace Game3;

	constexpr float scale = Game3::UI_SCALE;

	std::shared_ptr<TextInputWidget> & getTextInput(UIContext &);

	auto & getProgressBar(UIContext &ui) {
		static std::shared_ptr<ProgressBarWidget> bar = nullptr;

		if (!bar) {
			bar = std::make_shared<ProgressBarWidget>(scale, scale * 10, Color(1, 0, 0, 1), 0.5);

			auto drag = [&](Widget &widget, int x, int y) {
				const float scale = widget.getScale();
				const Rectangle &last = widget.getLastRectangle();
				const int width = last.width;
				const int height = last.height;

				if (!(width <= 2 * scale || x < scale || y < scale || x > width - scale || y > height - scale)) {
					const float progress = static_cast<float>(x - scale) / static_cast<float>(width - 2 * scale);
					bar->setProgress(progress);
					getTextInput(ui)->setText(ui, std::format("{:.1f}%", progress * 100));
				}

				return true;
			};

			bar->setOnDrag([drag](Widget &widget, UIContext &, int x, int y) {
				return drag(widget, x, y);
			});
		}

		return bar;
	}

	std::shared_ptr<TextInputWidget> & getTextInput(UIContext &ui) {
		static std::shared_ptr<TextInputWidget> input = nullptr;

		if (!input) {
			input = std::make_shared<TextInputWidget>(scale);
			input->setText(ui, "Hello from the crafting tab! This is some example text.");
			input->onSubmit = [&ui](TextInputWidget &input) {
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
					number = parseNumber<float>(text.raw()) / divisor;
				} catch (...) {
					input.setText(ui, "Invalid number.");
					return;
				}

				getProgressBar(ui)->setProgress(number);
			};
		}

		return input;
	}

	auto & getButton(UIContext &ui) {
		static std::shared_ptr<ButtonWidget> button = nullptr;

		if (!button) {
			button = std::make_shared<ButtonWidget>(scale, scale * 10);
			button->setText("Randomize");
			button->setOnClick([&, i = 0](Widget &, UIContext &, int, int, int) mutable {
				auto &input = getTextInput(ui);
				INFO("Clicked {} time(s). Text = \"{}\"", ++i, input->getText().raw());
				const float progress = threadContext.random(0.f, 1.f);
				getProgressBar(ui)->setProgress(progress);
				input->setText(ui, std::format("{:.1f}%", progress * 100));
				return true;
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

		auto &bar = getProgressBar(ui);
		if (bar->getLastRectangle().contains(x, y))
			bar->click(ui, mouse_button, x, y);
	}

	void CraftingTab::dragStart(int x, int y) {
		auto &input = getTextInput(ui);
		if (input->getLastRectangle().contains(x, y))
			input->dragStart(ui, x, y);

		auto &button = getButton(ui);
		if (button->getLastRectangle().contains(x, y))
			button->dragStart(ui, x, y);

		auto &bar = getProgressBar(ui);
		if (bar->getLastRectangle().contains(x, y))
			bar->dragStart(ui, x, y);
	}

	void CraftingTab::dragUpdate(int x, int y) {
		auto &input = getTextInput(ui);
		if (input->getLastRectangle().contains(x, y))
			input->dragUpdate(ui, x, y);

		auto &button = getButton(ui);
		if (button->getLastRectangle().contains(x, y))
			button->dragUpdate(ui, x, y);

		auto &bar = getProgressBar(ui);
		if (bar->getLastRectangle().contains(x, y))
			bar->dragUpdate(ui, x, y);
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
