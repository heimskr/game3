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

#include <cassert>
namespace Game3 {
	void CraftingTab::init() {
		INFO("CraftingTab::init()");
		assert(!bar);

		auto tab = shared_from_this();

		bar = std::make_shared<ProgressBarWidget>(scale, scale * 10, Color(1, 0, 0, 1), 0.5);

		auto bar_drag = [&](Widget &widget, int x, int y) {
			const float scale = widget.getScale();
			const Rectangle &last = widget.getLastRectangle();
			const int width = last.width;
			const int height = last.height;

			if (!(width <= 2 * scale || x < scale || y < scale || x > width - scale || y > height - scale)) {
				const float progress = static_cast<float>(x - scale) / static_cast<float>(width - 2 * scale);
				bar->setProgress(progress);
				input->setText(ui, std::format("{:.1f}%", progress * 100));
			}

			return true;
		};

		bar->setOnDrag([bar_drag](Widget &widget, UIContext &, int x, int y) {
			return bar_drag(widget, x, y);
		});

		bar->insertAtEnd(tab);

		input = std::make_shared<TextInputWidget>(scale);
		input->setText(ui, "Hello from the crafting tab! This is some example text.");
		input->onSubmit = [&](TextInputWidget &input, UIContext &ui) {
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

			bar->setProgress(number);
		};

		input->insertAtEnd(tab);

		button = std::make_shared<ButtonWidget>(scale, scale * 10);
		button->setText("Randomize");
		button->setOnClick([&, i = 0](Widget &, UIContext &, int, int, int) mutable {;
			INFO("Clicked {} time(s). Text = \"{}\"", ++i, input->getText().raw());
			const float progress = threadContext.random(0.f, 1.f);
			bar->setProgress(progress);
			input->setText(ui, std::format("{:.1f}%", progress * 100));
			return true;
		});

		button->insertAtEnd(tab);
	}

	void CraftingTab::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float) {
		input->render(ui, renderers, x, y, width, scale * TEXT_INPUT_HEIGHT_FACTOR);

		y += scale * (TEXT_INPUT_HEIGHT_FACTOR + 2);

		bar->render(ui, renderers, x + width / 4, y, width / 2, scale * 10);

		y += scale * 12;

		button->render(ui, renderers, x + scale * 2, y, -1, -1);
	}

	void CraftingTab::renderIcon(const RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/crafting.png"));
	}
}
