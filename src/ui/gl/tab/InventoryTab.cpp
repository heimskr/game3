#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "packet/SwapSlotsPacket.h"
#include "ui/gl/module/Module.h"
#include "ui/gl/tab/InventoryTab.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace {
	constexpr int INNER_SLOT_SIZE = 16;
	constexpr int OUTER_SLOT_SIZE = INNER_SLOT_SIZE * 5 / 4;
}

namespace Game3 {
	void InventoryTab::render(UIContext &ui, RendererContext &renderers) {
		innerRectangle = ui.scissorStack.getTop();

		PlayerPtr player = ui.getPlayer();

		if (!player)
			return;

		InventoryPtr inventory = player->getInventory(0);
		auto inventory_lock = inventory->sharedLock();

		const Slot slot_count = inventory->getSlotCount();
		assert(0 <= slot_count);

		const Slot active_slot = inventory->activeSlot;

		if (slotWidgets.size() != static_cast<size_t>(slot_count)) {
			slotWidgets.clear();
			for (Slot slot = 0; slot < slot_count; ++slot)
				slotWidgets.emplace_back(std::make_shared<ItemSlotWidget>((*inventory)[slot], slot, INNER_SLOT_SIZE, SLOT_SCALE, slot == active_slot));
		} else {
			for (Slot slot = 0; slot < slot_count; ++slot)
				slotWidgets[slot]->setStack((*inventory)[slot]);

			if (0 <= previousActive) {
				if (previousActive != active_slot) {
					slotWidgets.at(previousActive)->setActive(false);
					slotWidgets.at(active_slot)->setActive(true);
				}
			} else {
				slotWidgets.at(active_slot)->setActive(true);
			}
		}

		previousActive = active_slot;

		const int column_count = std::max<int>(1, innerRectangle.width / (OUTER_SLOT_SIZE * SLOT_SCALE));
		const double x_pad = (innerRectangle.width - column_count * (OUTER_SLOT_SIZE * SLOT_SCALE) + (OUTER_SLOT_SIZE - INNER_SLOT_SIZE) * SLOT_SCALE) / 2;

		int column = 0;
		double x = x_pad;
		double y = (OUTER_SLOT_SIZE - INNER_SLOT_SIZE) * SLOT_SCALE;

		for (const std::shared_ptr<ItemSlotWidget> &widget: slotWidgets) {
			widget->render(ui, renderers, x, y);

			x += OUTER_SLOT_SIZE * SLOT_SCALE;

			if (++column == column_count) {
				column = 0;
				x = x_pad;
				y += OUTER_SLOT_SIZE * SLOT_SCALE;
			}
		}
	}

	void InventoryTab::renderIcon(RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/inventory.png"));
	}

	void InventoryTab::click(int x, int y) {
		for (const std::shared_ptr<ItemSlotWidget> &widget: slotWidgets) {
			Rectangle rectangle = innerRectangle + widget->getLastRectangle();
			if (rectangle.contains(x, y) && widget->click(ui))
				break;
		}
	}

	void InventoryTab::dragStart(int x, int y) {
		for (const std::shared_ptr<ItemSlotWidget> &widget: slotWidgets) {
			Rectangle rectangle = innerRectangle + widget->getLastRectangle();
			if (rectangle.contains(x, y)) {
				ui.setDraggedWidget(widget->getDragStartWidget());
				break;
			}
		}
	}

	void InventoryTab::dragEnd(int x, int y) {
		auto dragged = std::dynamic_pointer_cast<ItemSlotWidget>(ui.getDraggedWidget());

		if (!dragged)
			return;

		for (const std::shared_ptr<ItemSlotWidget> &widget: slotWidgets) {
			Rectangle rectangle = innerRectangle + widget->getLastRectangle();
			if (rectangle.contains(x, y)) {
				ClientPlayerPtr player = ui.getGame()->getPlayer();
				const InventoryID inventory_id = player->getInventory(0)->index;
				player->send(SwapSlotsPacket(player->getGID(), player->getGID(), dragged->getSlot(), widget->getSlot(), inventory_id, inventory_id));
				ui.setDraggedWidget(nullptr);
				return;
			}
		}
	}

	void InventoryTab::setModule(std::shared_ptr<Module> new_module) {
		assert(new_module);
		auto lock = activeModule.uniqueLock();
		(activeModule.getBase() = std::move(new_module))->reset();
	}

	Module * InventoryTab::getModule(std::shared_lock<DefaultMutex> &lock) {
		if (activeModule)
			lock = activeModule.sharedLock();
		return activeModule.getBase().get();
	}

	Module * InventoryTab::getModule(std::unique_lock<DefaultMutex> &lock) {
		if (activeModule)
			lock = activeModule.uniqueLock();
		return activeModule.getBase().get();
	}

	void InventoryTab::removeModule() {
		activeModule.reset();
	}
}
