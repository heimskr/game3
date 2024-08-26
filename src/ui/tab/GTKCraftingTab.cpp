#include <iostream>

#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "packet/CraftPacket.h"
#include "recipe/CraftingRecipe.h"
#include "registry/Registries.h"
#include "threading/ThreadContext.h"
#include "ui/MainWindow.h"
#include "ui/gtk/CraftXDialog.h"
#include "ui/gtk/Util.h"
#include "ui/tab/GTKCraftingTab.h"
#include "ui/tab/GTKInventoryTab.h"
#include "util/Util.h"

namespace Game3 {
	GTKCraftingTab::GTKCraftingTab(MainWindow &main_window): GTKTab(main_window.notebook), mainWindow(main_window) {
		scrolled.set_vexpand(true);
		scrolled.add_css_class("crafting-tab");
		scrolled.set_child(vbox);

		gmenu = Gio::Menu::create();
		gmenu->append("Craft _1", "crafting_popup.craft_one");
		gmenu->append("Craft _X", "crafting_popup.craft_x");
		gmenu->append("Craft _All", "crafting_popup.craft_all");

		auto group = Gio::SimpleActionGroup::create();
		group->add_action("craft_one", [this] { craftOne(lastGame, lastRegistryID); });
		group->add_action("craft_x",   [this] { craftX  (lastGame, lastRegistryID); });
		group->add_action("craft_all", [this] { craftAll(lastGame, lastRegistryID); });

		mainWindow.insert_action_group("crafting_popup", group);

		popoverMenu.set_parent(mainWindow);
	}

	GTKCraftingTab::~GTKCraftingTab() {
		popoverMenu.unparent();
	}

	void GTKCraftingTab::update(const ClientGamePtr &game) {
		reset(game);
	}

	void GTKCraftingTab::reset(const ClientGamePtr &game) {
		if (!game) {
			lastGame.reset();
			removeChildren(vbox);
			widgets.clear();
			return;
		}

		ClientPlayerPtr player = game->getPlayer();

		if (!player)
			return;

		lastGame = game;

		// Perhaps I ought to use a grid.
		removeChildren(vbox);
		widgets.clear();

		const InventoryPtr inventory = player->getInventory(0);
		if (!inventory)
			return;

		auto lock = inventory->sharedLock();

		auto &recipe_registry = game->registries.get<CraftingRecipeRegistry>();
		auto registry_lock = recipe_registry.sharedLock();

		for (const auto &recipe: recipe_registry.items) {
			if (player->stationTypes.contains(recipe->stationType) && recipe->canCraft(inventory)) {
				auto hbox = std::make_unique<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
				auto left_vbox = std::make_unique<Gtk::Box>(Gtk::Orientation::VERTICAL);
				auto right_vbox = std::make_unique<Gtk::Box>(Gtk::Orientation::VERTICAL);
				Glib::ustring output_label_text;
				for (const ItemStackPtr &output: recipe->output) {
					auto fixed = std::make_unique<Gtk::Fixed>();
					auto image = std::make_unique<Gtk::Image>(output->getImage(*game));
					auto label = std::make_unique<Gtk::Label>(std::to_string(output->count));
					if (!output_label_text.empty())
						output_label_text += " + ";
					if (output->count != 1)
						output_label_text += std::to_string(output->count) + ' ';
					output_label_text += output->getTooltip();
					Glib::ustring tooltip = output->getTooltip();
					if (output->count != 1)
						tooltip += " \u00d7 " + std::to_string(output->count);
					label->set_tooltip_text(tooltip);
					label->set_xalign(1.f);
					label->set_yalign(1.f);
					image->set_size_request(GTKInventoryTab::TILE_SIZE - GTKInventoryTab::TILE_MAGIC, GTKInventoryTab::TILE_SIZE - GTKInventoryTab::TILE_MAGIC);
					label->set_size_request(GTKInventoryTab::TILE_SIZE - GTKInventoryTab::TILE_MAGIC, GTKInventoryTab::TILE_SIZE - GTKInventoryTab::TILE_MAGIC);
					fixed->put(*image, 0, 0);
					fixed->put(*label, 0, 0);
					left_vbox->append(*fixed);
					widgets.push_back(std::move(fixed));
					widgets.push_back(std::move(image));
					widgets.push_back(std::move(label));
				}

				auto output_label = std::make_unique<Gtk::Label>(output_label_text);
				output_label->set_xalign(0.f);
				output_label->add_css_class("output-label");
				right_vbox->append(*output_label);

				for (const CraftingRequirement &input: recipe->input) {
					std::unique_ptr<Gtk::Label> label;
					if (input.is<ItemStackPtr>()) {
						ItemStackPtr stack = input.get<ItemStackPtr>();
						Glib::ustring ending = stack->count == 0? " (not consumed)" : "";
						label = std::make_unique<Gtk::Label>((1 < stack->count? std::to_string(stack->count) + " \u00d7 " : "") + stack->getTooltip() + ending);
					} else {
						const auto &[attribute, count] = input.get<AttributeRequirement>();
						Glib::ustring ending = count == 0? " (not consumed)" : "";
						label = std::make_unique<Gtk::Label>((1 < count? std::to_string(count) + " \u00d7 " : "") + "any " + attribute.getPostPath() + ending);
					}
					label->set_xalign(0.f);
					label->add_css_class("input-label");
					right_vbox->append(*label);
					widgets.push_back(std::move(label));
				}

				right_vbox->add_css_class("right");
				hbox->add_css_class("recipe");
				hbox->append(*left_vbox);
				hbox->append(*right_vbox);
				vbox.append(*hbox);

				auto left_click = Gtk::GestureClick::create();
				left_click->set_button(1);
				left_click->signal_pressed().connect([this, game, hbox = hbox.get(), id = recipe->registryID](int n, double x, double y) {
					leftClick(game, hbox, id, n, x, y);
				});
				hbox->add_controller(left_click);

				auto right_click = Gtk::GestureClick::create();
				right_click->set_button(3);
				right_click->signal_pressed().connect([this, game, hbox = hbox.get(), id = recipe->registryID](int, double x, double y) {
					rightClick(game, hbox, id, x, y);
				});
				hbox->add_controller(right_click);

				widgets.push_back(std::move(output_label));
				widgets.push_back(std::move(left_vbox));
				widgets.push_back(std::move(right_vbox));
				widgets.push_back(std::move(hbox));
			}
		}
	}

	void GTKCraftingTab::craftOne(const ClientGamePtr &game, size_t registry_id) {
		game->getPlayer()->send(CraftPacket(threadContext.rng(), registry_id, 1));
	}

	void GTKCraftingTab::craftX(const ClientGamePtr &game, size_t registry_id) {
		auto dialog = std::make_unique<CraftXDialog>(mainWindow);
		dialog->signal_submit().connect([weak_game = std::weak_ptr(game), registry_id](int count) {
			if (0 < count)
				if (ClientGamePtr game = weak_game.lock())
					game->getPlayer()->send(CraftPacket(threadContext.rng(), registry_id, static_cast<uint64_t>(count)));
		});
		mainWindow.queueDialog(std::move(dialog));
	}

	void GTKCraftingTab::craftAll(const ClientGamePtr &game, size_t registry_id) {
		game->getPlayer()->send(CraftPacket(threadContext.rng(), registry_id, -1));
	}

	void GTKCraftingTab::leftClick(const ClientGamePtr &game, Gtk::Widget *, size_t registry_id, int n, double, double) {
		mainWindow.onBlur();
		if (n % 2 == 0)
			craftOne(game, registry_id);
	}

	void GTKCraftingTab::rightClick(const ClientGamePtr &game, Gtk::Widget *widget, size_t registry_id, double x, double y) {
		mainWindow.onBlur();

		do {
			const auto allocation = widget->get_allocation();
			x += allocation.get_x();
			y += allocation.get_y();
			widget = widget->get_parent();
		} while (widget);

		lastGame = game;
		lastRegistryID = registry_id;

		popoverMenu.set_pointing_to({int(x), int(y), 1, 1});
		popoverMenu.set_menu_model(gmenu);
		popoverMenu.set_has_arrow(true);
		popoverMenu.popup();
	}
}
