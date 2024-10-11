#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "graphics/RendererContext.h"
#include "packet/MoveSlotsPacket.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/Window.h"
#include "util/Defer.h"
#include "util/Math.h"

namespace {
	constexpr int getColumnCount(float width, std::size_t slot_count) {
		return std::min(std::min(10, std::max(1, static_cast<int>(slot_count))), std::max<int>(1, width / (Game3::OUTER_SLOT_SIZE * Game3::SLOT_SCALE)));
	}
}

namespace Game3 {
	namespace {
		ClientInventoryPtr getInventoryArgument(const std::any &any) {
			const InventoryModule::Argument *argument = std::any_cast<InventoryModule::Argument>(&any);
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

	InventoryModule::InventoryModule(UIContext &ui, const std::shared_ptr<ClientGame> &, const std::any &argument):
		InventoryModule(ui, getInventoryArgument(argument)) {}

	InventoryModule::InventoryModule(UIContext &ui, const std::shared_ptr<ClientInventory> &inventory):
		Module(ui, SLOT_SCALE), inventoryGetter(inventory? inventory->getGetter() : nullptr) {}

	void InventoryModule::init() {
		assert(inventoryGetter);
		InventoryPtr inventory = inventoryGetter->get();
		auto inventory_lock = inventory->sharedLock();
		const Slot slot_count = inventory->getSlotCount();
		assert(0 <= slot_count);
		const bool is_player = inventory->getOwner() == ui.getPlayer();
		const Slot active_slot = inventory->activeSlot;
		for (Slot slot = 0; slot < slot_count; ++slot)
			slotWidgets.emplace_back(std::make_shared<ItemSlot>(ui, inventory, (*inventory)[slot], slot, INNER_SLOT_SIZE, scale, is_player && slot == active_slot));
	}

	void InventoryModule::render(const RendererContext &renderers, float x, float y, float width, float height) {
		Widget::render(renderers, x, y, width, height);

		InventoryPtr inventory = inventoryGetter->get();
		auto inventory_lock = inventory->sharedLock();

		const Slot slot_count = inventory->getSlotCount();
		assert(0 <= slot_count);

		const bool is_player = inventory->getOwner() == ui.getPlayer();
		const Slot active_slot = inventory->activeSlot;

		if (slotWidgets.size() != static_cast<size_t>(slot_count)) {
			slotWidgets.clear();
			for (Slot slot = 0; slot < slot_count; ++slot)
				slotWidgets.emplace_back(std::make_shared<ItemSlot>(ui, inventory, (*inventory)[slot], slot, INNER_SLOT_SIZE, scale, is_player && slot == active_slot));
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

		const int column_count = getColumnCount(width, slotWidgets.size());
		const float x_pad = (width - column_count * (OUTER_SLOT_SIZE * scale) + SLOT_PADDING * scale) / 2;

		int column = 0;
		float slot_x = x_pad;
		float slot_y = topPadding * scale;

		for (const std::shared_ptr<ItemSlot> &widget: slotWidgets) {
			widget->render(renderers, x + slot_x, y + slot_y, -1, -1);

			slot_x += OUTER_SLOT_SIZE * scale;

			if (++column == column_count) {
				column = 0;
				slot_x = x_pad;
				slot_y += OUTER_SLOT_SIZE * scale;
			}
		}
	}

	bool InventoryModule::click(int button, int x, int y) {
		const Modifiers modifiers = ui.window.getModifiers();

		for (const std::shared_ptr<ItemSlot> &widget: slotWidgets) {
			if (!widget->contains(x, y)) {
				continue;
			}

			if (clickSlot(widget->getSlot(), modifiers)) {
				return true;
			}

			if (widget->click(button, x, y)) {
				return true;
			}
		}

		return false;
	}

	bool InventoryModule::dragStart(int x, int y) {
		for (const std::shared_ptr<ItemSlot> &widget: slotWidgets) {
			if (widget->contains(x, y)) {
				WidgetPtr dragged_widget = widget->getDragStartWidget();
				const bool out = dragged_widget != nullptr;
				ui.setDraggedWidget(std::move(dragged_widget));
				return out;
			}
		}

		return false;
	}

	bool InventoryModule::dragEnd(int x, int y) {
		auto dragged = std::dynamic_pointer_cast<ItemSlot>(ui.getDraggedWidget());

		if (!dragged) {
			return false;
		}

		for (const std::shared_ptr<ItemSlot> &widget: slotWidgets) {
			if (widget != dragged && widget->contains(x, y)) {
				ClientPlayerPtr player = ui.getPlayer();
				player->send(make<MoveSlotsPacket>(dragged->getOwnerGID(), widget->getOwnerGID(), dragged->getSlot(), widget->getSlot(), dragged->getInventory()->index, widget->getInventory()->index));
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
		if (slotWidgets.empty()) {
			minimum = natural = 0;
			return;
		}

		if (orientation == Orientation::Horizontal) {
			minimum = natural = (getColumnCount(for_width, slotWidgets.size()) * OUTER_SLOT_SIZE - SLOT_PADDING + topPadding) * scale;
		} else {
			minimum = natural = (updiv(slotWidgets.size(), getColumnCount(for_width, slotWidgets.size())) * OUTER_SLOT_SIZE - SLOT_PADDING + topPadding) * scale;
		}
	}

	std::shared_ptr<InventoryModule> InventoryModule::getPrimaryInventoryModule() {
		return std::static_pointer_cast<InventoryModule>(shared_from_this());
	}

	void InventoryModule::setTopPadding(float new_top_padding) {
		topPadding = new_top_padding;
	}

	float InventoryModule::getTopPadding() const {
		return topPadding;
	}

	InventoryPtr InventoryModule::getInventory() const {
		assert(inventoryGetter != nullptr);
		return inventoryGetter->get();
	}

	void InventoryModule::setOnSlotClick(std::function<bool(Slot, Modifiers)> function) {
		onSlotClick = std::move(function);
	}

	bool InventoryModule::clickSlot(Slot slot, Modifiers modifiers) {
		if (onSlotClick && onSlotClick(slot, modifiers)) {
			return true;
		}

		if (modifiers.onlyShift()) {
			ClientPlayerPtr player = ui.getPlayer();
			assert(player != nullptr);
			InventoryPtr inventory = *inventoryGetter;
			assert(inventory != nullptr);
			player->send(make<MoveSlotsPacket>(inventory->getOwner()->getGID(), player->getGID(), slot, -1, inventory->index, 0));
			return true;
		}

		return false;
	}
}
