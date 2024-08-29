#include "entity/ClientPlayer.h"
#include "game/Inventory.h"
#include "packet/SwapSlotsPacket.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "util/Defer.h"

namespace Game3 {
	void InventoryModule::render(UIContext &ui, RendererContext &renderers, float x, float y, float width, float height) {
		innerRectangle = ui.scissorStack.pushRelative(Rectangle(x, y, width, height));
		Defer pop([&ui] { ui.scissorStack.pop(); });

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

		const int column_count = std::min(10, std::max<int>(1, innerRectangle.width / (OUTER_SLOT_SIZE * SLOT_SCALE)));
		const float x_pad = (innerRectangle.width - column_count * (OUTER_SLOT_SIZE * SLOT_SCALE) + SLOT_PADDING * SLOT_SCALE) / 2;

		int column = 0;
		float slot_x = x_pad;
		float slot_y = SLOT_PADDING * SLOT_SCALE;

		for (const std::shared_ptr<ItemSlotWidget> &widget: slotWidgets) {
			widget->render(ui, renderers, slot_x, slot_y, -1, -1);

			slot_x += OUTER_SLOT_SIZE * SLOT_SCALE;

			if (++column == column_count) {
				column = 0;
				slot_x = x_pad;
				slot_y += OUTER_SLOT_SIZE * SLOT_SCALE;
			}
		}
	}

	bool InventoryModule::click(UIContext &ui, int x, int y) {
		for (const std::shared_ptr<ItemSlotWidget> &widget: slotWidgets) {
			Rectangle rectangle = innerRectangle + widget->getLastRectangle();
			if (rectangle.contains(x, y) && widget->click(ui, x, y))
				return true;
		}

		return false;
	}

	bool InventoryModule::dragStart(UIContext &ui, int x, int y) {
		for (const std::shared_ptr<ItemSlotWidget> &widget: slotWidgets) {
			Rectangle rectangle = innerRectangle + widget->getLastRectangle();
			if (rectangle.contains(x, y)) {
				ui.setDraggedWidget(widget->getDragStartWidget());
				return true;
			}
		}

		return false;
	}

	bool InventoryModule::dragEnd(UIContext &ui, int x, int y) {
		auto dragged = std::dynamic_pointer_cast<ItemSlotWidget>(ui.getDraggedWidget());

		if (!dragged)
			return false;

		for (const std::shared_ptr<ItemSlotWidget> &widget: slotWidgets) {
			Rectangle rectangle = innerRectangle + widget->getLastRectangle();
			if (rectangle.contains(x, y)) {
				ClientPlayerPtr player = ui.getPlayer();
				const InventoryID inventory_id = player->getInventory(0)->index;
				player->send(SwapSlotsPacket(player->getGID(), player->getGID(), dragged->getSlot(), widget->getSlot(), inventory_id, inventory_id));
				ui.setDraggedWidget(nullptr);
				break;
			}
		}

		return true;
	}
}
