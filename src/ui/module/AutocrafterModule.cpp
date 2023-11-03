#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "net/Buffer.h"
#include "recipe/CraftingRecipe.h"
#include "tileentity/Autocrafter.h"
#include "ui/gtk/UITypes.h"
#include "ui/gtk/Util.h"
#include "ui/module/AutocrafterModule.h"
#include "ui/module/ExternalInventoryModule.h"
#include "ui/tab/InventoryTab.h"
#include "ui/MainWindow.h"

namespace Game3 {
	AutocrafterModule::AutocrafterModule(std::shared_ptr<ClientGame> game_, const std::any &argument):
		AutocrafterModule(game_, std::dynamic_pointer_cast<Autocrafter>(std::any_cast<AgentPtr>(argument))) {}

	AutocrafterModule::AutocrafterModule(std::shared_ptr<ClientGame> game_, std::shared_ptr<Autocrafter> autocrafter_):
	game(std::move(game_)),
	autocrafter(std::move(autocrafter_)),
	inventoryModule(std::make_unique<ExternalInventoryModule>(game, std::static_pointer_cast<ClientInventory>(autocrafter->getInventory()))) {
		assert(autocrafter);
		vbox.set_hexpand();

		header.set_text(autocrafter->getName());
		header.set_margin(10);
		header.set_xalign(0.5);
		vbox.append(header);

		entry.set_placeholder_text("Target");
		{
			const auto &target = autocrafter->getTarget();
			auto lock = target.sharedLock();
			if (target.empty())
				entry.set_text({});
			else
				entry.set_text(target.str());

		}
		entry.set_margin(5);

		std::set<Identifier> item_names;
		for (const auto &recipe: game->registry<CraftingRecipeRegistry>())
			for (const ItemStack &stack: recipe->output)
				item_names.insert(stack.item->identifier);

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
	}

	Gtk::Widget & AutocrafterModule::getWidget() {
		return vbox;
	}

	void AutocrafterModule::reset() {
		inventoryModule->reset();
	}

	void AutocrafterModule::update() {
		inventoryModule->update();
	}

	void AutocrafterModule::onResize(int width) {
		inventoryModule->onResize(width);
	}

	std::optional<Buffer> AutocrafterModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
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

			if (source && source->getGID() == autocrafter->getGID()) {
				inventoryModule->handleMessage(source, name, data);
				MainWindow &window = game->getWindow();
				window.queue([&window] { window.removeModule(); });
			}

		} else if (name == "GetAgentGID") {

			return Buffer{autocrafter->getGID()};

		} else {
			return inventoryModule->handleMessage(source, name, data);
		}

		return {};
	}

	void AutocrafterModule::setInventory(std::shared_ptr<ClientInventory> inventory) {
		inventoryModule->setInventory(std::move(inventory));
	}

	void AutocrafterModule::setTarget(const std::string &target) {
		if (autocrafter)
			game->player->sendMessage(autocrafter, "SetTarget", target);
	}
}
