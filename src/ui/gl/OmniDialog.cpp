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
#include "ui/gl/tab/SettingsTab.h"
#include "ui/gl/tab/Tab.h"
#include "ui/gl/widget/Tooltip.h"
#include "ui/gl/Constants.h"
#include "ui/gl/OmniDialog.h"
#include "ui/gl/UIContext.h"
#include "util/Defer.h"

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
		settingsTab = std::make_shared<SettingsTab>(ui);
		tabs = {inventoryTab, craftingTab, settingsTab};
		activeTab = inventoryTab;
		tabRectangles.resize(tabs.size());

		for (const TabPtr &tab: tabs)
			tab->init(ui);
	}

	void OmniDialog::render(const RendererContext &renderers) {
		ScissorStack &stack = ui.scissorStack;

		Rectangle rectangle = getPosition();
		constexpr Color inner_color{0.88, 0.77, 0.55};
		const Rectangle original_rectangle = rectangle;
		RectangleRenderer &rectangler = renderers.rectangle;

		{
			auto saver = stack.pushRelative(rectangle, renderers);

			drawFrame(renderers, UI_SCALE, false, PIECES, inner_color);

			rectangle.x = 9 * UI_SCALE;
			rectangle.y = 9 * UI_SCALE;
			rectangle.width -= 18 * UI_SCALE;
			rectangle.height -= 18 * UI_SCALE;

			if (rectangle.height <= 0 || rectangle.width <= 0)
				return;

			rectangler.drawOnScreen(Color{0.6, 0.3, 0, 0.1}, rectangle);

			auto subsaver = stack.pushRelative(rectangle, renderers);

			if (activeTab)
				activeTab->render(ui, renderers, 0, 0, rectangle.width, rectangle.height);
		}

		auto saver = renderers.getSaver();

		for (int i = 0; const std::shared_ptr<Tab> &tab: tabs) {
			const int x_offset = (TOP_OFFSET + UNSCALED / 4) * i + 40;
			const int y_offset = tab == activeTab? 2 * UI_SCALE : 0;

			Rectangle tab_rectangle = original_rectangle + Rectangle(x_offset, UNSCALED * 5 / 4 - TOP_OFFSET + y_offset, TOP_OFFSET, TOP_OFFSET);

			{
				auto saver = stack.pushRelative(tab_rectangle, renderers);
				drawFrame(renderers, UI_SCALE / UNSCALE, true, TAB_PIECES, inner_color);
				tabRectangles.at(i) = tab_rectangle;
			}

			if (tab == activeTab) {
				auto saver = stack.pushRelative(original_rectangle, renderers);
				rectangler.drawOnScreen(inner_color, x_offset + UNSCALED, 0, TOP_OFFSET - UNSCALED * 2, 6 * UI_SCALE);

				renderers.singleSprite.drawOnScreen(cacheTexture("resources/gui/gui_merge_left.png", true), RenderOptions{
					.x = double(x_offset + UI_SCALE / UNSCALE),
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
			}

			auto saver = stack.pushRelative(tab_rectangle, renderers);
			tab->renderIcon(renderers);

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

	void OmniDialog::onClose() {
		ui.getTooltip()->hide();
		inventoryTab->removeModule();
	}

	bool OmniDialog::click(int button, int x, int y) {
		for (size_t i = 0; i < tabRectangles.size(); ++i) {
			if (tabRectangles[i].contains(x, y)) {
				activeTab = tabs.at(i);
				return true;
			}
		}

		if (!Dialog::click(button, x, y))
			return false;

		if (activeTab)
			activeTab->click(ui, button, x, y);

		return true;
	}

	bool OmniDialog::dragStart(int x, int y) {
		if (!Dialog::dragStart(x, y))
			return false;

		if (activeTab)
			activeTab->dragStart(ui, x, y);

		return true;
	}

	bool OmniDialog::dragUpdate(int x, int y) {
		if (!Dialog::dragUpdate(x, y))
			return false;

		if (activeTab)
			activeTab->dragUpdate(ui, x, y);

		return true;
	}

	bool OmniDialog::dragEnd(int x, int y) {
		if (!Dialog::dragEnd(x, y))
			return false;

		if (activeTab)
			activeTab->dragEnd(ui, x, y);

		return true;
	}

	bool OmniDialog::scroll(float x_delta, float y_delta, int x, int y) {
		if (!Dialog::scroll(x_delta, y_delta, x, y))
			return false;

		if (activeTab)
			activeTab->scroll(ui, x_delta, y_delta, x, y);

		return true;
	}
}
