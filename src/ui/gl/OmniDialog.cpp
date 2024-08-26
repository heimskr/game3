#include "game/ClientGame.h"
#include "graphics/ItemTexture.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "graphics/TextRenderer.h"
#include "ui/gl/module/Module.h"
#include "ui/gl/tab/CraftingTab.h"
#include "ui/gl/tab/InventoryTab.h"
#include "ui/gl/tab/Tab.h"
#include "ui/gl/Constants.h"
#include "ui/gl/OmniDialog.h"
#include "ui/gl/UIContext.h"

namespace {
	constexpr std::array<std::string_view, 8> PIECES{
		"resources/gui/gui_topleft.png", "resources/gui/gui_top.png", "resources/gui/gui_topright.png", "resources/gui/gui_right.png",
		"resources/gui/gui_bottomright.png", "resources/gui/gui_bottom.png", "resources/gui/gui_bottomleft.png", "resources/gui/gui_left.png",
	};

	constexpr std::array<std::string_view, 8> TAB_PIECES{
		"resources/gui/gui_topleft.png", "resources/gui/gui_top.png", "resources/gui/gui_topright.png", "resources/gui/gui_right.png",
		"resources/gui/gui_bottomright_empty.png", "resources/gui/gui_bottom_empty.png", "resources/gui/gui_bottomleft_empty.png", "resources/gui/gui_left.png",
	};
}

namespace Game3 {
	OmniDialog::OmniDialog(UIContext &ui): Dialog(ui) {
		inventoryTab = std::make_shared<InventoryTab>(ui);
		craftingTab = std::make_shared<CraftingTab>(ui);
		tabs = {inventoryTab, craftingTab};
		activeTab = inventoryTab;
		tabRectangles.resize(tabs.size());
	}

	void OmniDialog::render(RendererContext &renderers) {
		ScissorStack &stack = ui.scissorStack;

		auto saver = renderers.getSaver();

		Rectangle rectangle = getPosition();

		const Color inner_color{0.88, 0.77, 0.55};

		const Rectangle original_rectangle = rectangle;

		stack.pushRelative(rectangle);
		renderers.updateSize(rectangle.width, rectangle.height);
		drawFrame(renderers, SCALE, false, PIECES, inner_color);

		RectangleRenderer &rectangler = renderers.rectangle;

		rectangle.x = 9 * SCALE;
		rectangle.y = 9 * SCALE;
		rectangle.width -= 18 * SCALE;
		rectangle.height -= 18 * SCALE;

		if (rectangle.height <= 0 || rectangle.width <= 0)
			return;

		stack.pushRelative(rectangle);
		renderers.updateSize(rectangle.width, rectangle.height);

		rectangler.drawOnScreen(Color{0.6, 0.3, 0, 0.1}, 0, 0, 10000, 10000);

		if (activeTab)
			activeTab->render(ui, renderers);

		stack.pop();
		stack.pop();

		for (int i = 0; const std::shared_ptr<Tab> &tab: tabs) {
			const int x_offset = (TOP_OFFSET + UNSCALED / 4) * i + 40;

			{
				auto saver = renderers.getSaver();
				Rectangle tab_rectangle = original_rectangle + Rectangle{x_offset, UNSCALED * 5 / 4 - TOP_OFFSET, TOP_OFFSET, TOP_OFFSET};
				stack.pushRelative(tab_rectangle);
				renderers.updateSize(tab_rectangle.width, tab_rectangle.height);
				drawFrame(renderers, SCALE / UNSCALE, true, TAB_PIECES, inner_color);
				tabRectangles.at(i) = tab_rectangle;
				stack.pop();
			}

			if (tab == activeTab) {
				stack.pushRelative(original_rectangle);
				renderers.updateSize(original_rectangle.width, original_rectangle.height);
				rectangler.drawOnScreen(inner_color, x_offset + UNSCALED, 0, TOP_OFFSET - UNSCALED * 2, 6 * SCALE);

				renderers.singleSprite.drawOnScreen(cacheTexture("resources/gui/gui_merge_left.png", true), RenderOptions{
					.x = double(x_offset + SCALE / UNSCALE),
					.y = 0,
					.sizeX = -1,
					.sizeY = -1,
					.scaleX = 1,
					.scaleY = 1,
					.invertY = false,
				});

				renderers.singleSprite.drawOnScreen(cacheTexture("resources/gui/gui_merge_right.png", true), RenderOptions{
					.x = double(x_offset - UNSCALED + TOP_OFFSET),
					.y = 0,
					.sizeX = -1,
					.sizeY = -1,
					.scaleX = 1,
					.scaleY = 1,
					.invertY = false,
				});

				stack.pop();
			}

			auto saver = renderers.getSaver();
			Rectangle tab_rectangle = original_rectangle + Rectangle{x_offset, UNSCALED * 5 / 4 - TOP_OFFSET, TOP_OFFSET, TOP_OFFSET};
			stack.pushRelative(tab_rectangle);
			renderers.updateSize(tab_rectangle.width, tab_rectangle.height);
			tab->renderIcon(renderers);
			stack.pop();

			++i;
		};
	}

	Rectangle OmniDialog::getPosition() const {
		Rectangle rectangle = ui.scissorStack.getBase();
		rectangle.x = rectangle.width * X_FRACTION / 2;
		rectangle.y = rectangle.height * Y_FRACTION / 2 + TOP_OFFSET;
		rectangle.width *= (1. - X_FRACTION);
		rectangle.height *= (1. - Y_FRACTION);
		rectangle.height -= TOP_OFFSET;
		return rectangle;
	}

	bool OmniDialog::click(int x, int y) {
		for (size_t i = 0; i < tabRectangles.size(); ++i) {
			if (tabRectangles[i].contains(x, y)) {
				activeTab = tabs.at(i);
				return true;
			}
		}

		if (!Dialog::click(x, y))
			return false;

		if (activeTab)
			activeTab->click(x, y);

		return true;
	}

	bool OmniDialog::dragStart(int x, int y) {
		if (!Dialog::dragStart(x, y))
			return false;

		if (activeTab)
			activeTab->dragStart(x, y);

		return true;
	}

	bool OmniDialog::dragEnd(int x, int y) {
		if (!Dialog::dragEnd(x, y))
			return false;

		if (activeTab)
			activeTab->dragEnd(x, y);

		return true;
	}
}
