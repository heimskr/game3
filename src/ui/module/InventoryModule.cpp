#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "packet/MoveSlotsPacket.h"
#include "ui/gtk/ItemSlot.h"
#include "ui/gtk/UITypes.h"
#include "ui/gtk/Util.h"
#include "ui/module/InventoryModule.h"
#include "ui/tab/InventoryTab.h"
#include "ui/MainWindow.h"
#include "util/Demangle.h"

namespace Game3 {
	InventoryModule::InventoryModule(std::shared_ptr<ClientGame> game_, const std::any &argument, const GmenuFn &gmenu_fn):
		InventoryModule(std::move(game_), getInventory(argument), gmenu_fn) {}

	InventoryModule::InventoryModule(std::shared_ptr<ClientGame> game_, std::shared_ptr<ClientInventory> inventory_, const GmenuFn &gmenu_fn):
	game(std::move(game_)),
	inventory(std::move(inventory_)) {
		assert(inventory);
		label.set_hexpand();
		flowBox.set_hexpand();
		flowBox.set_vexpand(false);
		flowBox.set_max_children_per_line(20);
		flowBox.set_can_focus(false);
		flowBox.set_selection_mode(Gtk::SelectionMode::NONE);
		// These take guints so this feels kinda wrong.
		flowBox.set_row_spacing(-2);
		flowBox.set_column_spacing(-2);
		hbox.append(label);
		vbox.append(hbox);
		vbox.append(flowBox);

		gmenu = Gio::Menu::create();
		if (gmenu_fn) {
			gmenu_fn(*this, gmenu);
		} else {
			gmenu->append("_Drop", "inventory_popup.drop");
			gmenu->append("D_iscard", "inventory_popup.discard");
		}

		popoverMenu.set_parent(vbox);
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

	InventoryID InventoryModule::getInventoryIndex(const std::any &any) {
		const Argument *argument = std::any_cast<Argument>(&any);
		if (!argument)
			throw std::invalid_argument("Invalid std::any argument given to InventoryModule: " + demangle(any.type().name()));
		return argument->index;
	}

	Gtk::Widget & InventoryModule::getWidget() {
		return vbox;
	}

	void InventoryModule::reset() {
		removeChildren(flowBox);
		itemSlots.clear();
		lastSlotCount = -1;
		update();
	}

	void InventoryModule::update() {
		if (!name.empty()) {
			label.set_text(name);
			label.show();
		}

		repopulate();
	}

	void InventoryModule::onResize(int width) {
		tabWidth = width;
		update();
	}

	void InventoryModule::setInventory(std::shared_ptr<ClientInventory> new_inventory) {
		inventory = std::move(new_inventory);
		update();
	}

	std::optional<Buffer> InventoryModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &) {
		if (name == "TileEntityRemoved") {

			if (source && source->getGID() == inventory->getOwner()->getGID()) {
				MainWindow &window = game->getWindow();
				window.queue([&window] { window.removeModule(); });
			}

		} else if (name == "GetAgentGID") {

			return Buffer{inventory->getOwner()->getGID()};

		}

		return {};
	}

	bool InventoryModule::addCSSClass(const Glib::ustring &css_class, Slot slot) {
		if (0 <= slot && slot < std::ssize(itemSlots)) {
			itemSlots.at(slot)->add_css_class(css_class);
			return true;
		}

		return false;
	}

	void InventoryModule::removeCSSClass(const Glib::ustring &css_class, Slot slot) {
		if (slot == -1) {
			for (const auto &item_slot: itemSlots)
				item_slot->remove_css_class(css_class);
		} else {
			itemSlots.at(slot)->remove_css_class(css_class);
		}
	}

	int InventoryModule::gridWidth() const {
		return tabWidth / (InventoryTab::TILE_SIZE + 2 * InventoryTab::TILE_MARGIN);
	}

	void InventoryModule::populate() {
		assert(inventory);
		auto lock = inventory->sharedLock();

		lastSlotCount = inventory->slotCount;
		itemSlots.clear();

		for (Slot slot = 0; slot < lastSlotCount; ++slot) {
			auto item_slot = std::make_unique<ItemSlot>(game, slot, inventory);

			if (ItemStack *stack = (*inventory)[slot])
				item_slot->setStack(*stack);

			item_slot->setLeftClick([this, slot, item_slot = item_slot.get()](Modifiers modifiers, int n, double x, double y) {
				leftClick(item_slot, n, slot, modifiers, x, y);
			});

			item_slot->setGmenu(gmenu);

			// item_slot->setRightClick([this, slot, item_slot = item_slot.get()](Modifiers modifiers, int n, double x, double y) {
			// 	rightClick(item_slot, n, slot, modifiers, x, y);
			// });

			flowBox.append(*item_slot);
			itemSlots.push_back(std::move(item_slot));
		}
	}

	void InventoryModule::repopulate() {
		assert(inventory);
		auto lock = inventory->sharedLock();

		if (inventory->slotCount.load() != lastSlotCount) {
			populate();
			return;
		}

		lastSlotCount = inventory->slotCount;

		for (Slot slot = 0; slot < lastSlotCount; ++slot) {
			ItemStack *stack = (*inventory)[slot];
			ItemSlot &item_slot = *itemSlots.at(slot);

			if (stack)
				item_slot.setStack(*stack);
			else
				item_slot.reset();
		}
	}

	void InventoryModule::leftClick(Gtk::Widget *, int, Slot slot, Modifiers modifiers, double, double) {
		if (!game || !modifiers.onlyShift() || !inventory->contains(slot))
			return;

		InventoryPtr player_inventory = game->player->getInventory(0);
		if (!player_inventory)
			return;

		AgentPtr owner = inventory->weakOwner.lock();
		if (!owner)
			return;

		game->player->send(MoveSlotsPacket(owner->getGID(), game->player->getGID(), slot, -1, inventory->index, 0));
	}

	// void InventoryModule::rightClick(Gtk::Widget *widget, int, Slot slot, Modifiers, double x, double y) {
	// 	// mainWindow.onBlur();

	// 	if (!inventory->contains(slot))
	// 		return;

	// 	const auto allocation = widget->get_allocation();
	// 	x += allocation.get_x();
	// 	y += allocation.get_y();

	// 	popoverMenu.set_has_arrow(true);
	// 	popoverMenu.set_pointing_to({int(x), int(y), 1, 1});
	// 	popoverMenu.set_menu_model(gmenu);
	// 	lastSlot = slot;
	// 	popoverMenu.popup();
	// }
}
