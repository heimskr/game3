#include "entity/ClientPlayer.h"
#include "game/ClientInventory.h"
#include "graphics/RendererContext.h"
#include "packet/SwapSlotsPacket.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "util/Defer.h"
#include "util/Math.h"

namespace Game3 {
	namespace {
		constexpr int getColumnCount(float width) {
			return std::min(10, std::max<int>(1, width / (OUTER_SLOT_SIZE * SLOT_SCALE)));
		}
	}

	InventoryModule::InventoryModule(std::shared_ptr<ClientGame> game, const std::any &argument):
		InventoryModule(std::move(game), getInventory(argument)) {}

	InventoryModule::InventoryModule(std::shared_ptr<ClientGame>, const std::shared_ptr<ClientInventory> &inventory):
		inventoryGetter(inventory->getGetter()) {}

	void InventoryModule::render(UIContext &ui, RendererContext &renderers, float x, float y, float width, float height) {
		Widget::render(ui, renderers, x, y, width, height);

		auto saver = renderers.getSaver();
		innerRectangle = ui.scissorStack.pushAbsolute(Rectangle(x, y, width, height));
		renderers.updateSize(innerRectangle.width, innerRectangle.height);
		Defer pop([&] { ui.scissorStack.pop(); });

		InventoryPtr inventory = inventoryGetter->get();
		auto inventory_lock = inventory->sharedLock();

		const Slot slot_count = inventory->getSlotCount();
		assert(0 <= slot_count);

		const bool is_player = inventory->getOwner() == ui.getPlayer();
		const Slot active_slot = inventory->activeSlot;

		if (slotWidgets.size() != static_cast<size_t>(slot_count)) {
			slotWidgets.clear();
			for (Slot slot = 0; slot < slot_count; ++slot)
				slotWidgets.emplace_back(std::make_shared<ItemSlotWidget>(inventory, (*inventory)[slot], slot, INNER_SLOT_SIZE, SLOT_SCALE, is_player && slot == active_slot));
		} else {
			for (Slot slot = 0; slot < slot_count; ++slot)
				slotWidgets[slot]->setStack((*inventory)[slot]);

			if (is_player) {
				if (0 <= previousActive) {
					if (previousActive != active_slot) {
						slotWidgets.at(previousActive)->setActive(false);
						slotWidgets.at(active_slot)->setActive(true);
					}
				} else {
					slotWidgets.at(active_slot)->setActive(true);
				}
			}
		}

		previousActive = active_slot;

		const int column_count = getColumnCount(innerRectangle.width);
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
		if (!getLastRectangle().contains(x, y))
			return false;

		for (const std::shared_ptr<ItemSlotWidget> &widget: slotWidgets) {
			Rectangle rectangle = innerRectangle + widget->getLastRectangle();
			if (rectangle.contains(x, y) && widget->click(ui, x, y))
				return true;
		}

		return false;
	}

	bool InventoryModule::dragStart(UIContext &ui, int x, int y) {
		if (!getLastRectangle().contains(x, y))
			return false;

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
		if (!getLastRectangle().contains(x, y))
			return false;

		auto dragged = std::dynamic_pointer_cast<ItemSlotWidget>(ui.getDraggedWidget());

		if (!dragged)
			return false;

		for (const std::shared_ptr<ItemSlotWidget> &widget: slotWidgets) {
			Rectangle rectangle = innerRectangle + widget->getLastRectangle();
			if (rectangle.contains(x, y)) {
				ClientPlayerPtr player = ui.getPlayer();
				player->send(SwapSlotsPacket(dragged->getOwnerGID(), widget->getOwnerGID(), dragged->getSlot(), widget->getSlot(), dragged->getInventory()->index, widget->getInventory()->index));
				ui.setDraggedWidget(nullptr);
				break;
			}
		}

		return true;
	}

	float InventoryModule::calculateHeight(RendererContext &, float available_width, float) {
		return updiv(slotWidgets.size(), getColumnCount(available_width)) * OUTER_SLOT_SIZE * SLOT_SCALE;
	}

	ClientInventoryPtr InventoryModule::getInventory(const std::any &any) {
		const Argument *argument = std::any_cast<Argument>(&any);
		if (!argument) {
			const AgentPtr *agent = std::any_cast<AgentPtr>(&any);
			if (!agent)
				throw std::invalid_argument("Invalid std::any argument given to InventoryModule: " + demangle(any.type().name()));
			auto has_inventory = std::dynamic_pointer_cast<HasInventory>(*agent);
			if (!has_inventory)
				throw std::invalid_argument("Agent supplied to InventoryModule isn't castable to HasInventory");
			return std::dynamic_pointer_cast<ClientInventory>(has_inventory->getInventory(0));
		}
		const auto [agent, index] = *argument;
		return std::dynamic_pointer_cast<ClientInventory>(std::dynamic_pointer_cast<HasInventory>(agent)->getInventory(index));
	}
}