#include "entity/ClientPlayer.h"
#include "game/ClientInventory.h"
#include "graphics/RendererContext.h"
#include "packet/SwapSlotsPacket.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "util/Defer.h"
#include "util/Math.h"

namespace {
	constexpr int getColumnCount(float width) {
		return std::min(10, std::max<int>(1, width / (Game3::OUTER_SLOT_SIZE * Game3::SLOT_SCALE)));
	}
}

namespace Game3 {

	InventoryModule::InventoryModule(std::shared_ptr<ClientGame> game, const std::any &argument):
		InventoryModule(std::move(game), getInventory(argument)) {}

	InventoryModule::InventoryModule(std::shared_ptr<ClientGame>, const std::shared_ptr<ClientInventory> &inventory):
		Module(SLOT_SCALE), inventoryGetter(inventory->getGetter()) {}

	void InventoryModule::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		Widget::render(ui, renderers, x, y, width, height);

		InventoryPtr inventory = inventoryGetter->get();
		auto inventory_lock = inventory->sharedLock();

		const Slot slot_count = inventory->getSlotCount();
		assert(0 <= slot_count);

		const bool is_player = inventory->getOwner() == ui.getPlayer();
		const Slot active_slot = inventory->activeSlot;

		if (slotWidgets.size() != static_cast<size_t>(slot_count)) {
			slotWidgets.clear();
			for (Slot slot = 0; slot < slot_count; ++slot)
				slotWidgets.emplace_back(std::make_shared<ItemSlot>(inventory, (*inventory)[slot], slot, INNER_SLOT_SIZE, SLOT_SCALE, is_player && slot == active_slot));
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

		const int column_count = getColumnCount(width);
		const float x_pad = (width - column_count * (OUTER_SLOT_SIZE * SLOT_SCALE) + SLOT_PADDING * SLOT_SCALE) / 2;

		int column = 0;
		float slot_x = x_pad;
		float slot_y = SLOT_PADDING * SLOT_SCALE;

		for (const std::shared_ptr<ItemSlot> &widget: slotWidgets) {
			widget->render(ui, renderers, x + slot_x, y + slot_y, -1, -1);

			slot_x += OUTER_SLOT_SIZE * SLOT_SCALE;

			if (++column == column_count) {
				column = 0;
				slot_x = x_pad;
				slot_y += OUTER_SLOT_SIZE * SLOT_SCALE;
			}
		}
	}

	bool InventoryModule::click(UIContext &ui, int button, int x, int y) {
		for (const std::shared_ptr<ItemSlot> &widget: slotWidgets)
			if (widget->getLastRectangle().contains(x, y) && widget->click(ui, button, x, y))
				return true;

		return false;
	}

	bool InventoryModule::dragStart(UIContext &ui, int x, int y) {
		for (const std::shared_ptr<ItemSlot> &widget: slotWidgets) {
			if (widget->getLastRectangle().contains(x, y)) {
				WidgetPtr dragged_widget = widget->getDragStartWidget();
				const bool out = dragged_widget != nullptr;
				ui.setDraggedWidget(std::move(dragged_widget));
				return out;
			}
		}

		return false;
	}

	bool InventoryModule::dragEnd(UIContext &ui, int x, int y) {
		auto dragged = std::dynamic_pointer_cast<ItemSlot>(ui.getDraggedWidget());

		if (!dragged)
			return false;

		for (const std::shared_ptr<ItemSlot> &widget: slotWidgets) {
			if (widget->getLastRectangle().contains(x, y)) {
				ClientPlayerPtr player = ui.getPlayer();
				player->send(SwapSlotsPacket(dragged->getOwnerGID(), widget->getOwnerGID(), dragged->getSlot(), widget->getSlot(), dragged->getInventory()->index, widget->getInventory()->index));
				ui.setDraggedWidget(nullptr);
				break;
			}
		}

		return true;
	}

	SizeRequestMode InventoryModule::getRequestMode() const {
		return SizeRequestMode::HeightForWidth;
	}

	void InventoryModule::measure(const RendererContext &, Orientation orientation, float for_width, float, float &minimum, float &natural) {
		if (orientation == Orientation::Horizontal) {
			minimum = getColumnCount(for_width) * OUTER_SLOT_SIZE * scale;
			natural = for_width;
		} else {
			minimum = natural = updiv(slotWidgets.size(), getColumnCount(for_width)) * OUTER_SLOT_SIZE * scale;
		}
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
