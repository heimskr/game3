#include <iostream>

#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "item/Tool.h"
#include "packet/MoveSlotsPacket.h"
#include "packet/SetHeldItemPacket.h"
#include "ui/MainWindow.h"
#include "ui/gtk/EntryDialog.h"
#include "ui/gtk/NumericEntry.h"
#include "ui/gtk/UITypes.h"
#include "ui/gtk/Util.h"
#include "ui/tab/InventoryTab.h"
#include "ui/module/InventoryModule.h"
#include "ui/module/Module.h"
#include "util/Util.h"

namespace Game3 {
	InventoryTab::InventoryTab(MainWindow &main_window): Tab(main_window.notebook), mainWindow(main_window) {
		scrolled.set_child(vbox);
		scrolled.set_hexpand();
		scrolled.set_vexpand();

		auto group = Gio::SimpleActionGroup::create();

		group->add_action("hold_left", [this] {
			if (lastGame)
				lastGame->player->send(SetHeldItemPacket(true, lastSlot));
			else
				WARN(__FILE__ << ':' << __LINE__ << ": no lastGame");
		});

		group->add_action("hold_right", [this] {
			if (lastGame)
				lastGame->player->send(SetHeldItemPacket(false, lastSlot));
			else
				WARN(__FILE__ << ':' << __LINE__ << ": no lastGame");
		});

		group->add_action("drop", [this] {
			if (lastGame)
				lastGame->player->getInventory(0)->drop(lastSlot);
			else
				WARN(__FILE__ << ':' << __LINE__ << ": no lastGame");
		});

		group->add_action("discard", [this] {
			if (lastGame)
				lastGame->player->getInventory(0)->discard(lastSlot);
			else
				WARN(__FILE__ << ':' << __LINE__ << ": no lastGame");
		});

		mainWindow.insert_action_group("inventory_popup", group);
		popoverMenu.set_parent(vbox);

		vbox.set_hexpand();
		vbox.set_vexpand();
	}

	void InventoryTab::onResize(const std::shared_ptr<ClientGame> &) {
		const int new_width = scrolled.get_width();
		if (new_width != lastWidth && currentModule) {
			lastWidth = new_width;
			currentModule->onResize(new_width);
		}
	}

	void InventoryTab::update(const std::shared_ptr<ClientGame> &game) {
		if (!game || !game->player)
			return;

		lastGame = game;

		mainWindow.queue([this, game] {
			updateInventory(game);
			auto lock = currentModule.trySharedLock();
			if (currentModule)
				currentModule->update();
		});
	}

	void InventoryTab::reset(const std::shared_ptr<ClientGame> &game) {
		if (!game) {
			clear();
			lastGame = nullptr;
			if (inventoryModule) {
				vbox.remove(inventoryModule->getWidget());
				inventoryModule.reset();
			}
			return;
		}

		if (!game->player)
			return;

		lastGame = game;

		mainWindow.queue([this, game] {
			clear();
			updateInventory(game);
			auto lock = currentModule.sharedLock();
			if (currentModule)
				currentModule->reset();
		});
	}

	void InventoryTab::clear() {
		if (inventoryModule)
			inventoryModule->reset();
	}

	void InventoryTab::populate(std::shared_ptr<ClientInventory> inventory) {
		if (!inventoryModule)
			return;

		if (inventory != inventoryModule->getInventory()) {
			inventoryModule->setInventory(inventory);
		} else {
			inventoryModule->update();
		}
	}

	void InventoryTab::setModule(std::shared_ptr<Module> module_) {
		assert(module_);
		removeModule();
		auto lock = currentModule.uniqueLock();
		currentModule.std::shared_ptr<Module>::operator=(std::move(module_));
		vbox.append(currentModule->getWidget());
		currentModule->onResize(vbox.get_width());
		currentModule->reset();
	}

	Module & InventoryTab::getModule() const {
		assert(currentModule);
		return *currentModule;
	}

	Module * InventoryTab::getModule(std::shared_lock<DefaultMutex> &lock) {
		if (currentModule)
			lock = currentModule.sharedLock();
		return currentModule.get();
	}

	Module * InventoryTab::getModule(std::unique_lock<DefaultMutex> &lock) {
		if (currentModule)
			lock = currentModule.uniqueLock();
		return currentModule.get();
	}

	void InventoryTab::removeModule() {
		auto lock = currentModule.uniqueLock();
		if (currentModule) {
			vbox.remove(currentModule->getWidget());
			currentModule.reset();
		}
	}

	GlobalID InventoryTab::getExternalGID() const {
		throw std::logic_error("InventoryTab::getExternalGID() needs to be replaced");
	}

	void InventoryTab::slotClicked(Slot slot, bool is_right_click, Modifiers modifiers) {
		if (is_right_click) {
			lastSlot = slot;
		} else {
			leftClick(slot, modifiers);
		}
	}

	void InventoryTab::activeSlotSet() {
		updatePlayerClasses(lastGame);
	}

	int InventoryTab::gridWidth() const {
		return scrolled.get_width() / (TILE_SIZE + 2 * TILE_MARGIN);
	}

	void InventoryTab::leftClick(Slot slot, Modifiers modifiers) {
		mainWindow.onBlur();

		if (!lastGame)
			return;

		if (modifiers.onlyShift()) {
			shiftClick(lastGame, slot);
		} else {
			lastGame->player->getInventory(0)->setActive(slot, false);
		}
	}

	void InventoryTab::shiftClick(const std::shared_ptr<ClientGame> &game, Slot slot) {
		if (!game)
			return;

		InventoryPtr inventory = game->player->getInventory(0);
		if (!inventory || !inventory->contains(slot))
			return;

		std::unique_lock<DefaultMutex> lock;
		Module *module_ = getModule(lock);
		if (!module_)
			return;

		if (module_->handleShiftClick(inventory, slot))
			return;

		std::shared_ptr<InventoryModule> external_module = module_->getPrimaryInventoryModule();
		if (!external_module)
			return;

		InventoryPtr external_inventory = external_module->getInventory();
		if (!external_inventory)
			return;

		AgentPtr owner = external_inventory->weakOwner.lock();
		if (!owner)
			return;

		game->player->send(MoveSlotsPacket(game->player->getGID(), owner->getGID(), slot, -1, 0, external_inventory->index));
	}

	void InventoryTab::updatePlayerClasses(const std::shared_ptr<ClientGame> &game) {
		if (!inventoryModule)
			return;

		const Slot active_slot = game->player->getInventory(0)->activeSlot;

		inventoryModule->removeCSSClass("active-slot");
		inventoryModule->addCSSClass("active-slot", active_slot);
	}

	void InventoryTab::gmenuSetup(InventoryModule &, Glib::RefPtr<Gio::Menu> gmenu) {
		gmenu->append("Hold (_Left)", "inventory_popup.hold_left");
		gmenu->append("Hold (_Right)", "inventory_popup.hold_right");
		gmenu->append("_Drop", "inventory_popup.drop");
		gmenu->append("D_iscard", "inventory_popup.discard");
	}

	void InventoryTab::updateInventory(const ClientGamePtr &game) {
		if (const InventoryPtr inventory = game->player->getInventory(0)) {
			auto client_inventory = std::static_pointer_cast<ClientInventory>(inventory);
			if (!inventoryModule) {
				inventoryModule.emplace(game, client_inventory, this, sigc::mem_fun(*this, &InventoryTab::gmenuSetup));
				vbox.prepend(inventoryModule->getWidget());
			}
			populate(client_inventory);
		}
	}
}
