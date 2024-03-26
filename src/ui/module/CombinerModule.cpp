#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "net/Buffer.h"
#include "recipe/CombinerRecipe.h"
#include "tileentity/Combiner.h"
#include "ui/gtk/DragSource.h"
#include "ui/gtk/Util.h"
#include "ui/module/CombinerModule.h"
#include "ui/module/EnergyLevelModule.h"
#include "ui/module/InventoryModule.h"
#include "ui/tab/InventoryTab.h"
#include "ui/MainWindow.h"

namespace Game3 {
	CombinerModule::CombinerModule(std::shared_ptr<ClientGame> game_, const std::any &argument):
		CombinerModule(game_, std::dynamic_pointer_cast<Combiner>(std::any_cast<AgentPtr>(argument))) {}

	CombinerModule::CombinerModule(std::shared_ptr<ClientGame> game_, std::shared_ptr<Combiner> combiner_):
	game(std::move(game_)),
	combiner(std::move(combiner_)),
	inventoryModule(std::make_shared<InventoryModule>(game, std::static_pointer_cast<ClientInventory>(combiner->getInventory(0)))),
	energyModule(std::make_shared<EnergyLevelModule>(game, std::static_pointer_cast<Agent>(combiner), false)) {
		assert(combiner);
		vbox.set_hexpand();

		header.set_text(combiner->getName());
		header.set_margin(10);
		header.set_xalign(0.5);
		vbox.append(header);

		entry.set_placeholder_text("Target");
		{
			const auto &target = combiner->getTarget();
			auto lock = target.sharedLock();
			if (target.empty())
				entry.set_text({});
			else
				entry.set_text(target.str());
		}
		entry.set_margin(5);

		std::set<Identifier> item_names;
		for (const auto &[id, recipe]: game->registry<CombinerRecipeRegistry>())
			item_names.insert(id);

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
		vbox.append(inventoryModule->getWidget());
		vbox.append(energyModule->getWidget());
	}

	Gtk::Widget & CombinerModule::getWidget() {
		return vbox;
	}

	void CombinerModule::reset() {
		inventoryModule->reset();
		energyModule->reset();
	}

	void CombinerModule::update() {
		inventoryModule->update();
		energyModule->update();
	}

	void CombinerModule::onResize(int width) {
		inventoryModule->onResize(width);
		energyModule->onResize(width);
	}

	std::optional<Buffer> CombinerModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "TargetSet") {

			auto *buffer = std::any_cast<Buffer>(&data);
			assert(buffer != nullptr);
			const bool success = buffer->take<bool>();
			const Identifier id = buffer->take<Identifier>();

			if (success) {
				entry.set_text(id.str());
				entry.set_position(-1);
			} else {
				entry.set_text({});
			}

		} else if (name == "TileEntityRemoved") {

			if (source && source->getGID() == combiner->getGID()) {
				inventoryModule->handleMessage(source, name, data);
				MainWindow &window = game->getWindow();
				window.queue([&window] { window.removeModule(); });
			}

		} else if (name == "GetAgentGID") {

			return Buffer{combiner->getGID()};

		} else if (name == "UpdateEnergy") {

			return energyModule->handleMessage(source, name, data);

		} else {
			return inventoryModule->handleMessage(source, name, data);
		}

		return {};
	}

	void CombinerModule::setInventory(std::shared_ptr<ClientInventory> inventory) {
		inventoryModule->setInventory(std::move(inventory));
	}

	void CombinerModule::setTarget(const std::string &target) {
		if (combiner)
			game->getPlayer()->sendMessage(combiner, "SetTarget", target);
	}
}
