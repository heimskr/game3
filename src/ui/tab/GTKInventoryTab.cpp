#include <iostream>

#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "item/Tool.h"
#include "packet/MoveSlotsPacket.h"
#include "packet/SetHeldItemPacket.h"
#include "packet/UseItemPacket.h"
#include "ui/MainWindow.h"
#include "ui/gtk/NumericEntry.h"
#include "ui/gtk/DragSource.h"
#include "ui/gtk/Util.h"
#include "ui/tab/GTKInventoryTab.h"
#include "ui/module/GTKModule.h"
#include "ui/module/GTKInventoryModule.h"
#include "util/Util.h"

namespace Game3 {
	GTKInventoryTab::GTKInventoryTab(MainWindow &main_window): GTKTab(main_window.notebook), mainWindow(main_window) {
		scrolled.set_child(vbox);
		scrolled.set_hexpand();
		scrolled.set_vexpand();

		vbox.set_spacing(0);
		actionBox.set_hexpand(true);
		actionBox.set_halign(Gtk::Align::CENTER);
		actionBox.set_margin_top(5);
		actionBox.set_margin_bottom(5);

		motion = Gtk::EventControllerMotion::create();
		motion->signal_motion().connect([this](double, double) {
			lastModifiers = Modifiers(motion->get_current_event_state());
		});
		scrolled.add_controller(motion);

		auto use_function = [this](Slot slot, Modifiers) {
			lastGame->getPlayer()->send(UseItemPacket(slot, Modifiers{}));
		};

		auto hold_left_function = [this](Slot slot, Modifiers) {
			lastGame->getPlayer()->send(SetHeldItemPacket(true, slot));
		};

		auto hold_right_function = [this](Slot slot, Modifiers) {
			lastGame->getPlayer()->send(SetHeldItemPacket(false, slot));
		};

		auto drop_function = [this](Slot slot, Modifiers) {
			lastGame->getPlayer()->getInventory(0)->drop(slot);
		};

		auto discard_function = [this](Slot slot, Modifiers modifiers) {
			InventoryPtr inventory = lastGame->getPlayer()->getInventory(0);
			auto lock = inventory->uniqueLock();
			if (modifiers.onlyShift()) {
				ItemStackPtr stack = (*inventory)[slot];
				if (!stack)
					return;
				std::vector<Slot> slots_to_remove;
				inventory->iterate([&](const ItemStackPtr &iterated, Slot iterated_slot) -> bool {
					if (iterated->canMerge(*stack))
						slots_to_remove.push_back(iterated_slot);
					return false;
				});
				for (Slot slot_to_remove: slots_to_remove)
					inventory->discard(slot_to_remove);
			} else {
				inventory->discard(slot);
			}
		};

		initAction(holdLeftAction,  "pan-start-symbolic",  "Hold Left",  hold_left_function);
		initAction(holdRightAction, "pan-end-symbolic",    "Hold Right", hold_right_function);
		initAction(dropAction,      "pan-down-symbolic",   "Drop",       drop_function);
		initAction(discardAction,   "user-trash-symbolic", "Discard",    discard_function);

		group = Gio::SimpleActionGroup::create();

		group->add_action("use", [this, use_function] {
			if (lastGame)
				use_function(lastSlot, Modifiers{});
			else
				WARN("{}:{}: no lastGame", __FILE__, __LINE__);
		});

		group->add_action("hold_left", [this, hold_left_function] {
			if (lastGame)
				hold_left_function(lastSlot, Modifiers{});
			else
				WARN("{}:{}: no lastGame", __FILE__, __LINE__);
		});

		group->add_action("hold_right", [this, hold_right_function] {
			if (lastGame)
				hold_right_function(lastSlot, Modifiers{});
			else
				WARN("{}:{}: no lastGame", __FILE__, __LINE__);
		});

		group->add_action("drop", [this, drop_function] {
			if (lastGame)
				drop_function(lastSlot, Modifiers{});
			else
				WARN("{}:{}: no lastGame", __FILE__, __LINE__);
		});

		group->add_action("discard", [this, discard_function] {
			if (lastGame)
				discard_function(lastSlot, Modifiers{});
			else
				WARN("{}:{}: no lastGame", __FILE__, __LINE__);
		});

		mainWindow.insert_action_group("inventory_popup", group);

		vbox.set_hexpand();
		vbox.set_vexpand();
	}

	GTKInventoryTab::~GTKInventoryTab() = default;

	void GTKInventoryTab::onResize(const std::shared_ptr<ClientGame> &) {
		const int new_width = scrolled.get_width();
		if (new_width != lastWidth && currentModule) {
			lastWidth = new_width;
			currentModule->onResize(new_width);
		}
	}

	void GTKInventoryTab::update(const std::shared_ptr<ClientGame> &game) {
		if (!game || !game->getPlayer())
			return;

		lastGame = game;

		mainWindow.queue([this, game] {
			updateInventory(game);
			auto lock = currentModule.trySharedLock();
			if (currentModule)
				currentModule->update();
		});
	}

	void GTKInventoryTab::reset(const std::shared_ptr<ClientGame> &game) {
		if (!game) {
			clear();
			lastGame = nullptr;
			if (inventoryModule) {
				vbox.remove(inventoryModule->getWidget());
				vbox.remove(actionBox);
				inventoryModule.reset();
			}
			return;
		}

		if (!game->getPlayer())
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

	void GTKInventoryTab::clear() {
		if (inventoryModule)
			inventoryModule->reset();
	}

	void GTKInventoryTab::populate(std::shared_ptr<ClientInventory> inventory) {
		if (!inventoryModule)
			return;

		if (inventory != inventoryModule->getInventory()) {
			inventoryModule->setInventory(inventory);
		} else {
			inventoryModule->update();
		}

		updatePlayerClasses(lastGame);
	}

	void GTKInventoryTab::setModule(std::shared_ptr<GTKModule> module_) {
		assert(module_);
		removeModule();
		auto lock = currentModule.uniqueLock();
		currentModule.std::shared_ptr<GTKModule>::operator=(std::move(module_));
		vbox.append(currentModule->getWidget());
		currentModule->onResize(vbox.get_width());
		currentModule->reset();
	}

	GTKModule & GTKInventoryTab::getModule() const {
		assert(currentModule);
		return *currentModule;
	}

	GTKModule * GTKInventoryTab::getModule(std::shared_lock<DefaultMutex> &lock) {
		if (currentModule)
			lock = currentModule.sharedLock();
		return currentModule.get();
	}

	GTKModule * GTKInventoryTab::getModule(std::unique_lock<DefaultMutex> &lock) {
		if (currentModule)
			lock = currentModule.uniqueLock();
		return currentModule.get();
	}

	void GTKInventoryTab::removeModule() {
		auto lock = currentModule.uniqueLock();
		if (currentModule) {
			vbox.remove(currentModule->getWidget());
			currentModule.reset();
		}
	}

	GlobalID GTKInventoryTab::getExternalGID() const {
		throw std::logic_error("InventoryTab::getExternalGID() needs to be replaced");
	}

	void GTKInventoryTab::slotClicked(Slot slot, bool is_right_click, Modifiers modifiers) {
		if (is_right_click) {
			lastSlot = slot;
		} else {
			leftClick(slot, modifiers);
		}
	}

	void GTKInventoryTab::slotDoubleClicked(Slot slot) {
		InventoryPtr inventory;
		PlayerPtr player;
		{
			if (!lastGame)
				return;
			player = lastGame->getPlayer();
			if (player)
				inventory = player->getInventory(0);
		}

		{
			auto lock = inventory->sharedLock();
			if (!inventory->contains(slot))
				return;
		}

		player->send(UseItemPacket(slot, Modifiers{}));
	}

	void GTKInventoryTab::activeSlotSet() {
		updatePlayerClasses(lastGame);
	}

	int GTKInventoryTab::gridWidth() const {
		return scrolled.get_width() / (TILE_SIZE + 2 * TILE_MARGIN);
	}

	void GTKInventoryTab::leftClick(Slot slot, Modifiers modifiers) {
		mainWindow.onBlur();

		if (!lastGame)
			return;

		if (modifiers.onlyShift()) {
			shiftClick(lastGame, slot);
		} else {
			lastGame->getPlayer()->getInventory(0)->setActive(slot, false);
		}
	}

	void GTKInventoryTab::shiftClick(const std::shared_ptr<ClientGame> &game, Slot slot) {
		if (!game)
			return;

		InventoryPtr inventory = game->getPlayer()->getInventory(0);
		if (!inventory || !inventory->contains(slot))
			return;

		std::unique_lock<DefaultMutex> lock;
		GTKModule *module_ = getModule(lock);
		if (!module_)
			return;

		if (module_->handleShiftClick(inventory, slot))
			return;

		std::shared_ptr<GTKInventoryModule> external_module = module_->getPrimaryInventoryModule();
		if (!external_module)
			return;

		InventoryPtr external_inventory = external_module->getInventory();
		if (!external_inventory)
			return;

		AgentPtr owner = external_inventory->weakOwner.lock();
		if (!owner)
			return;

		game->getPlayer()->send(MoveSlotsPacket(game->getPlayer()->getGID(), owner->getGID(), slot, -1, 0, external_inventory->index));
	}

	void GTKInventoryTab::updatePlayerClasses(const std::shared_ptr<ClientGame> &game) {
		if (!inventoryModule)
			return;

		const Slot active_slot = game->getPlayer()->getInventory(0)->activeSlot;

		inventoryModule->removeCSSClass("active-slot");
		inventoryModule->addCSSClass("active-slot", active_slot);
	}

	void GTKInventoryTab::gmenuSetup(GTKInventoryModule &module_, Glib::RefPtr<Gio::Menu> gmenu, Slot slot, const ItemStackPtr &stack) {
		if (!stack || !stack->item->populateMenu(module_.getInventory(), slot, stack, gmenu, group))
			gmenu->append("_Use", "inventory_popup.use");
		gmenu->append("Hold (_Left)", "inventory_popup.hold_left");
		gmenu->append("Hold (_Right)", "inventory_popup.hold_right");
		gmenu->append("_Drop", "inventory_popup.drop");
		gmenu->append("D_iscard", "inventory_popup.discard");
	}

	void GTKInventoryTab::updateInventory(const ClientGamePtr &game) {
		if (const InventoryPtr inventory = game->getPlayer()->getInventory(0)) {
			auto client_inventory = std::static_pointer_cast<ClientInventory>(inventory);

			if (!inventoryModule) {
				inventoryModule.emplace(game, client_inventory, this, sigc::mem_fun(*this, &GTKInventoryTab::gmenuSetup));
				inventoryModule->setShowLabel(false);
				vbox.prepend(inventoryModule->getWidget());
				vbox.prepend(actionBox);
			} else {
				inventoryModule->setInventory(client_inventory);
			}

			populate(client_inventory);
		}
	}

	void GTKInventoryTab::initAction(Gtk::Image &action, const Glib::ustring &icon, const Glib::ustring &tooltip, std::function<void(Slot, Modifiers)> function) {
		action.set_margin_start(5);
		action.set_margin_end(5);
		action.set_from_icon_name(icon);
		action.set_icon_size(Gtk::IconSize::LARGE);
		action.set_tooltip_text(tooltip);

		auto target = Gtk::DropTarget::create(Glib::Value<DragSource>::value_type(), Gdk::DragAction::MOVE);
		target->signal_drop().connect([this, function = std::move(function)](const Glib::ValueBase &base, double, double) {
			if (base.gobj()->g_type != Glib::Value<DragSource>::value_type())
				return false;

			const auto &value = static_cast<const Glib::Value<DragSource> &>(base);
			const DragSource source = value.get();
			if (lastGame && lastGame->getPlayer() && *source.inventory == *lastGame->getPlayer()->getInventory(0))
				function(source.slot, lastModifiers);
			return true;
		}, false);

		action.add_controller(target);
		actionBox.append(action);
	}
}