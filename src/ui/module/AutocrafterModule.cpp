#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "net/Buffer.h"
#include "packet/SwapSlotsPacket.h"
#include "recipe/CraftingRecipe.h"
#include "tileentity/Autocrafter.h"
#include "ui/gtk/DragSource.h"
#include "ui/gtk/Util.h"
#include "ui/module/AutocrafterModule.h"
#include "ui/module/EnergyLevelModule.h"
#include "ui/module/GTKInventoryModule.h"
#include "ui/tab/GTKInventoryTab.h"
#include "ui/MainWindow.h"

namespace Game3 {
	AutocrafterModule::AutocrafterModule(std::shared_ptr<ClientGame> game_, const std::any &argument):
		AutocrafterModule(game_, std::dynamic_pointer_cast<Autocrafter>(std::any_cast<AgentPtr>(argument))) {}

	AutocrafterModule::AutocrafterModule(std::shared_ptr<ClientGame> game_, std::shared_ptr<Autocrafter> autocrafter_):
	game(std::move(game_)),
	autocrafter(std::move(autocrafter_)),
	inventoryModule(std::make_shared<GTKInventoryModule>(game, std::static_pointer_cast<ClientInventory>(autocrafter->getInventory(0)))),
	stationInventoryModule(std::make_shared<GTKInventoryModule>(game, std::static_pointer_cast<ClientInventory>(autocrafter->getInventory(1)))),
	energyModule(std::make_shared<EnergyLevelModule>(game, std::static_pointer_cast<Agent>(autocrafter), false)) {
		assert(autocrafter);
		vbox.set_hexpand();

		header.set_text(autocrafter->getName());
		header.set_margin(10);
		header.set_xalign(0.5);
		vbox.append(header);

		entry.set_placeholder_text("Target");
		updateEntry();
		entry.set_margin(5);

		game->getWindow().addYield(entry);

		std::set<Identifier> item_names;
		for (const auto &recipe: game->registry<CraftingRecipeRegistry>())
			for (const ItemStackPtr &stack: recipe->output)
				item_names.insert(stack->item->identifier);

		store = Gtk::ListStore::create(columns);
		for (const Identifier &item_name: item_names) {
			auto iter = store->append();
			(*iter)[columns.itemName] = item_name.str();
		}

		completion = Gtk::EntryCompletion::create();
		completion->set_model(store);
		completion->set_popup_completion();
		completion->set_minimum_key_length(1);
		completion->set_text_column(columns.itemName);
		entry.set_completion(completion);

		entry.signal_activate().connect([this] { setTarget(entry.get_text().raw()); });
		completion->signal_match_selected().connect([this](const Gtk::TreeIter<Gtk::TreeRow> &iter) {
			setTarget(Glib::ustring((*iter)[columns.itemName]).raw());
			return false;
		}, false);

		entry.signal_changed().connect([this] {
			entry.remove_css_class("equation_error");
			entry.remove_css_class("equation_success");
		});

		vbox.append(entry);
		vbox.append(stationInventoryModule->getWidget());
		vbox.append(inventoryModule->getWidget());
		vbox.append(energyModule->getWidget());
	}

	Gtk::Widget & AutocrafterModule::getWidget() {
		return vbox;
	}

	void AutocrafterModule::reset() {
		stationInventoryModule->reset();
		inventoryModule->reset();
		energyModule->reset();
		updateEntry();
	}

	void AutocrafterModule::update() {
		stationInventoryModule->update();
		inventoryModule->update();
		energyModule->update();
		updateEntry();
	}

	void AutocrafterModule::onResize(int width) {
		stationInventoryModule->onResize(width);
		inventoryModule->onResize(width);
		energyModule->onResize(width);
	}

	std::optional<Buffer> AutocrafterModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "TargetSet") {

			auto *buffer = std::any_cast<Buffer>(&data);
			assert(buffer != nullptr);
			const bool success = buffer->take<bool>();
			Identifier id = buffer->take<Identifier>();

			if (success) {
				entry.set_text(id.str());
				entry.set_position(-1);
			} else {
				entry.set_text({});
			}

			autocrafter->setTarget(std::move(id));

		} else if (name == "TileEntityRemoved") {

			if (source && source->getGID() == autocrafter->getGID()) {
				stationInventoryModule->handleMessage(source, name, data);
				inventoryModule->handleMessage(source, name, data);
				MainWindow &window = game->getWindow();
				window.queue([&window] { window.removeModule(); });
			}

		} else if (name == "GetAgentGID") {

			return Buffer{autocrafter->getGID()};

		} else if (name == "UpdateEnergy") {

			return energyModule->handleMessage(source, name, data);

		} else {
			return inventoryModule->handleMessage(source, name, data);
		}

		return {};
	}

	void AutocrafterModule::setInventory(std::shared_ptr<ClientInventory> inventory) {
		if (inventory->index == 0)
			inventoryModule->setInventory(std::move(inventory));
		else if (inventory->index == 1)
			stationInventoryModule->setInventory(std::move(inventory));
		else
			throw std::invalid_argument("Can't set AutocrafterModule inventory at index " + std::to_string(inventory->index));
	}


	void AutocrafterModule::updateEntry() {
		const auto &target = autocrafter->getTarget();
		auto lock = target.sharedLock();
		if (target.empty())
			entry.set_text({});
		else
			entry.set_text(target.str());
	}

	void AutocrafterModule::setTarget(const std::string &target) {
		if (autocrafter)
			game->getPlayer()->sendMessage(autocrafter, "SetTarget", target);
	}
}
