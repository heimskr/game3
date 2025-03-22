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
#include "ui/gl/dialog/OmniDialog.h"
#include "ui/gl/UIContext.h"
#include "util/Defer.h"

namespace {
	constexpr std::array<std::string_view, 8> TAB_PIECES{
		"resources/gui/gui_topleft.png", "resources/gui/gui_top.png", "resources/gui/gui_topright.png", "resources/gui/gui_right.png",
		"resources/gui/gui_bottomright_empty.png", "resources/gui/gui_bottom_empty.png", "resources/gui/gui_bottomleft_empty.png", "resources/gui/gui_left.png",
	};

	constexpr double X_FRACTION = 0.2;
	constexpr double Y_FRACTION = 0.2;
}

namespace Game3 {
	OmniDialog::OmniDialog(UIContext &ui, float scale):
		Dialog(ui) {
			inventoryTab = std::make_shared<InventoryTab>(ui, scale);
			craftingTab = std::make_shared<CraftingTab>(ui, scale);
			settingsTab = std::make_shared<SettingsTab>(ui, scale);
			tabs = {inventoryTab, craftingTab, settingsTab};
			activeTab = inventoryTab;
			tabRectangles.resize(tabs.size());

			for (const TabPtr &tab: tabs) {
				tab->init();
			}

			activeTab->onFocus();
		}

	void OmniDialog::render(const RendererContext &renderers) {
		ScissorStack &stack = ui.scissorStack;

		Rectangle rectangle = getPosition();
		const Rectangle original_rectangle = rectangle;
		RectangleRenderer &rectangler = renderers.rectangle;

		{
			auto saver = stack.pushRelative(rectangle, renderers);

			ui.drawFrame(renderers, scale, false, FRAME_PIECES, DEFAULT_BACKGROUND_COLOR);

			rectangle.x = 9 * scale;
			rectangle.y = 9 * scale;
			rectangle.width -= 18 * scale;
			rectangle.height -= 18 * scale;

			if (rectangle.height <= 0 || rectangle.width <= 0) {
				return;
			}

			rectangler.drawOnScreen(Color{0.6, 0.3, 0, 0.1}, rectangle);

			auto subsaver = stack.pushRelative(rectangle, renderers);

			if (activeTab) {
				activeTab->render(renderers, 0, 0, rectangle.width, rectangle.height);
			}
		}

		auto saver = renderers.getSaver();

		for (int i = 0; const std::shared_ptr<Tab> &tab: tabs) {
			const int x_offset = (TOP_OFFSET + UNSCALED / 4) * i + 40;
			const int y_offset = tab == activeTab? 2 * scale : 0;

			Rectangle tab_rectangle = original_rectangle + Rectangle(x_offset, UNSCALED * 5 / 4 - TOP_OFFSET + y_offset, TOP_OFFSET, TOP_OFFSET);

			{
				auto saver = stack.pushRelative(tab_rectangle, renderers);
				ui.drawFrame(renderers, scale / UNSCALE, true, TAB_PIECES, DEFAULT_BACKGROUND_COLOR);
				tabRectangles.at(i) = tab_rectangle;
			}

			if (tab == activeTab) {
				auto saver = stack.pushRelative(original_rectangle, renderers);
				rectangler.drawOnScreen(DEFAULT_BACKGROUND_COLOR, x_offset + UNSCALED, 0, TOP_OFFSET - UNSCALED * 2, 6 * scale);

				renderers.singleSprite.drawOnScreen(cacheTexture("resources/gui/gui_merge_left.png", true), RenderOptions{
					.x = double(x_offset + scale / UNSCALE),
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
		activeTab->onBlur();
	}

	bool OmniDialog::click(int button, int x, int y, Modifiers modifiers) {
		if (!getPosition().contains(x, y)) {
			return false;
		}

		if (activeTab && activeTab->click(button, x, y, modifiers)) {
			return true;
		}

		return Dialog::click(button, x, y, modifiers);
	}

	bool OmniDialog::mouseDown(int button, int x, int y, Modifiers modifiers) {
		mouseDownPosition.emplace(x, y);

		if (!getPosition().contains(x, y)) {
			return false;
		}

		if (activeTab && activeTab->mouseDown(button, x, y, modifiers)) {
			return true;
		}

		return Dialog::mouseDown(button, x, y, modifiers);
	}

	bool OmniDialog::mouseUp(int button, int x, int y, Modifiers modifiers) {
		if (mouseDownPosition) {
			const auto [mouse_down_x, mouse_down_y] = *mouseDownPosition;
			mouseDownPosition.reset();

			for (size_t i = 0; i < tabRectangles.size(); ++i) {
				if (tabRectangles[i].contains(x, y) && tabRectangles[i].contains(mouse_down_x, mouse_down_y)) {
					const TabPtr &clicked_tab = tabs.at(i);
					if (clicked_tab != activeTab) {
						activeTab->onBlur();
						activeTab = clicked_tab;
						activeTab->onFocus();
					}
					return true;
				}
			}
		}

		if (!getPosition().contains(x, y)) {
			return false;
		}

		if (activeTab && activeTab->mouseUp(button, x, y, modifiers)) {
			return true;
		}

		return Dialog::mouseUp(button, x, y, modifiers);
	}

	bool OmniDialog::dragStart(int x, int y) {
		if (!getPosition().contains(x, y)) {
			return false;
		}

		if (activeTab && activeTab->dragStart(x, y)) {
			return true;
		}

		return Dialog::dragStart(x, y);
	}

	bool OmniDialog::dragUpdate(int x, int y) {
		if (!getPosition().contains(x, y)) {
			return false;
		}

		if (activeTab && activeTab->dragUpdate(x, y)) {
			return true;
		}

		return Dialog::dragUpdate(x, y);
	}

	bool OmniDialog::dragEnd(int x, int y) {
		if (!getPosition().contains(x, y)) {
			return false;
		}

		if (activeTab && activeTab->dragEnd(x, y)) {
			return true;
		}

		return Dialog::dragEnd(x, y);
	}

	bool OmniDialog::scroll(float x_delta, float y_delta, int x, int y, Modifiers modifiers) {
		if (!getPosition().contains(x, y)) {
			return false;
		}

		if (activeTab && activeTab->scroll(x_delta, y_delta, x, y, modifiers)) {
			return true;
		}

		return Dialog::scroll(x_delta, y_delta, x, y, modifiers);
	}

	bool OmniDialog::hidesHotbar() const {
		return true;
	}

	void OmniDialog::updateModule() {
		std::unique_lock<DefaultMutex> lock;

		if (Module *module_ = inventoryTab->getModule(lock)) {
			module_->update();
		}
	}
}
