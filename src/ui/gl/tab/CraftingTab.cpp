#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "threading/ThreadContext.h"
#include "ui/gl/tab/CraftingTab.h"
#include "ui/gl/widget/BoxWidget.h"
#include "ui/gl/widget/ButtonWidget.h"
#include "ui/gl/widget/IconButtonWidget.h"
#include "ui/gl/widget/IconWidget.h"
#include "ui/gl/widget/ProgressBarWidget.h"
#include "ui/gl/widget/TextInputWidget.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "util/Util.h"

#include <cassert>

namespace Game3 {
	void CraftingTab::init() {
		assert(!bar);

		auto tab = shared_from_this();

		box = std::make_shared<BoxWidget>(scale);

		input = std::make_shared<TextInputWidget>(scale);
		input->setFixedHeight(scale * TEXT_INPUT_HEIGHT_FACTOR);
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

		bar = std::make_shared<ProgressBarWidget>(scale, Color(1, 0, 0, 1), 0.5);
		bar->setFixedHeight(10 * scale);
		bar->setOnDragStart([this](Widget &, UIContext &ui, int, int) {
			ui.addDragUpdater(bar);
			return true;
		});
		bar->setOnDragUpdate([this](Widget &widget, UIContext &, int x, int) {
			if (!widget.isDragging())
				return false;

			const Rectangle &last = widget.getLastRectangle();
			const float scale = widget.getScale();

			if (last.width > 2 * scale) {
				x = std::min<int>(last.width - scale, std::max<int>(x, scale));
				const float progress = static_cast<float>(x - scale) / static_cast<float>(last.width - 2 * scale);
				bar->setProgress(progress);
				button->setFixedHeight(scale * lerp(6.f, 32.f, progress));
				input->setText(ui, std::format("{:.1f}%", progress * 100));
			}

			return true;
		});

		iconButton = std::make_shared<IconButtonWidget>(scale);
		iconButton->setFixedHeight(scale * 12);
		iconButton->setIconTexture(cacheTexture("resources/gui/randomize.png"));
		iconButton->setOnClick([this](Widget &, UIContext &ui, int, int, int) {
			const float height = threadContext.random(6.f, 32.f);
			button->setFixedHeight(scale * height);
			input->setText(ui, std::format("{} * {} = {}", scale, height, scale * height));
			return true;
		});

		button = std::make_shared<ButtonWidget>(scale);
		button->setFixedHeight(scale * 10);
		button->setText("Randomize");
		button->setOnClick([&](Widget &, UIContext &, int, int, int) {
			const float progress = threadContext.random(0.f, 1.f);
			bar->setProgress(progress);
			input->setText(ui, std::format("{:.1f}%", progress * 100));
			return true;
		});

		icon = std::make_shared<IconWidget>(scale);
		icon->setIconTexture(cacheTexture("resources/gui/settings.png"));
		icon->setFixedSize(scale * 6);
		icon->setOnClick([](Widget &, UIContext &, int, int, int) {
			INFO("Icon clicked");
			return true;
		});

		icon->insertAtEnd(tab);
		box->insertAtEnd(tab);
		bar->insertAtEnd(box);
		input->insertAtEnd(box);
		iconButton->insertAtEnd(box);
		button->insertAtEnd(box);
	}

	void CraftingTab::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		box->render(ui, renderers, x, y, width, height);

		const float icon_size = icon->getFixedHeight();
		Rectangle rect = input->getLastRectangle() - ui.scissorStack.getTop().rectangle;
		rect.x += rect.width - 2 * scale - icon_size;
		rect.y += (rect.height - icon_size) / 2;
		rect.width = icon_size;
		rect.height = icon_size;

		icon->render(ui, renderers, rect);
	}

	void CraftingTab::renderIcon(const RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/crafting.png"));
	}
}
