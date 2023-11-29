#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "packet/MoveSlotsPacket.h"
#include "ui/gtk/UITypes.h"
#include "ui/gtk/Util.h"
#include "ui/module/ExternalInventoryModule.h"
#include "ui/tab/InventoryTab.h"
#include "ui/MainWindow.h"
#include "util/Demangle.h"

namespace Game3 {
	ExternalInventoryModule::ExternalInventoryModule(std::shared_ptr<ClientGame> game_, const std::any &argument):
		ExternalInventoryModule(std::move(game_), getInventory(argument)) {}

	ExternalInventoryModule::ExternalInventoryModule(std::shared_ptr<ClientGame> game_, std::shared_ptr<ClientInventory> inventory_):
	game(std::move(game_)),
	inventory(std::move(inventory_)) {
		assert(inventory);
		label.set_hexpand();
		grid.set_hexpand();
		hbox.append(label);
		vbox.append(hbox);
		vbox.append(grid);

		gmenu = Gio::Menu::create();
		gmenu->append("_Drop", "inventory_popup.drop");
		gmenu->append("D_iscard", "inventory_popup.discard");

		popoverMenu.set_parent(vbox);

		source = Gtk::DragSource::create();
		source->set_actions(Gdk::DragAction::MOVE);
		source->signal_prepare().connect([this](double x, double y) -> Glib::RefPtr<Gdk::ContentProvider> {
			auto *item = grid.pick(x, y);

			if (dynamic_cast<Gtk::Fixed *>(item->get_parent()))
				item = item->get_parent();

			if (auto *label = dynamic_cast<Gtk::Label *>(item)) {
				if (label->get_text().empty())
					return nullptr;
			} else if (!dynamic_cast<Gtk::Fixed *>(item))
				return nullptr;

			Glib::Value<DragSource> value;
			value.init(value.value_type());
			value.set({widgetMap.at(item), inventory, inventory->index});
			return Gdk::ContentProvider::create(value);
		}, false);

		auto target = Gtk::DropTarget::create(Glib::Value<DragSource>::value_type(), Gdk::DragAction::MOVE);
		target->signal_drop().connect([this](const Glib::ValueBase &base, double x, double y) {
			if (base.gobj()->g_type != Glib::Value<DragSource>::value_type())
				return false;

			const auto &value = static_cast<const Glib::Value<DragSource> &>(base);
			auto *destination = grid.pick(x, y);

			if (destination != nullptr && destination != &grid) {
				if (dynamic_cast<Gtk::Fixed *>(destination->get_parent()))
					destination = destination->get_parent();

				const DragSource source = value.get();
				game->player->send(MoveSlotsPacket(source.inventory->getOwner()->getGID(), inventory->getOwner()->getGID(), source.slot, widgetMap.at(destination), source.index, inventory->index));
			}

			return true;
		}, false);

		grid.add_controller(source);
		grid.add_controller(target);
	}

	ClientInventoryPtr ExternalInventoryModule::getInventory(const std::any &any) {
		const Argument *argument = std::any_cast<Argument>(&any);
		if (!argument) {
			const AgentPtr *agent = std::any_cast<AgentPtr>(&any);
			if (!agent)
				throw std::invalid_argument("Invalid std::any argument given to ExternalInventoryModule: " + demangle(any.type().name()));
			auto has_inventory = std::dynamic_pointer_cast<HasInventory>(*agent);
			if (!has_inventory)
				throw std::invalid_argument("Agent supplied to ExternalInventoryModule isn't castable to HasInventory");
			return std::dynamic_pointer_cast<ClientInventory>(has_inventory->getInventory(0));
		}
		const auto [agent, index] = *argument;
		return std::dynamic_pointer_cast<ClientInventory>(std::dynamic_pointer_cast<HasInventory>(agent)->getInventory(index));
	}

	InventoryID ExternalInventoryModule::getInventoryIndex(const std::any &any) {
		const Argument *argument = std::any_cast<Argument>(&any);
		if (!argument)
			throw std::invalid_argument("Invalid std::any argument given to ExternalInventoryModule: " + demangle(any.type().name()));
		return argument->index;
	}

	Gtk::Widget & ExternalInventoryModule::getWidget() {
		return vbox;
	}

	void ExternalInventoryModule::reset() {
		clickGestures.clear();
		widgetMap.clear();
		removeChildren(grid);
		widgets.clear();
		widgetsBySlot.clear();
		update();
	}

	void ExternalInventoryModule::update() {
		if (!name.empty()) {
			label.set_text(name);
			label.show();
		}

		populate();
	}

	void ExternalInventoryModule::onResize(int width) {
		tabWidth = width;
		update();
	}

	void ExternalInventoryModule::setInventory(std::shared_ptr<ClientInventory> new_inventory) {
		inventory = std::move(new_inventory);
		update();
	}

	std::optional<Buffer> ExternalInventoryModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &) {
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

	int ExternalInventoryModule::gridWidth() const {
		return tabWidth / (InventoryTab::TILE_SIZE + 2 * InventoryTab::TILE_MARGIN);
	}

	void ExternalInventoryModule::populate() {
		assert(inventory);
		const int grid_width = gridWidth();
		const int tile_size  = InventoryTab::TILE_SIZE <= tabWidth? tabWidth / (tabWidth / InventoryTab::TILE_SIZE) : InventoryTab::TILE_SIZE;

		for (Slot slot = 0; slot < inventory->slotCount; ++slot) {
			const int row    = slot / grid_width;
			const int column = slot % grid_width;
			std::unique_ptr<Gtk::Widget> widget_ptr;

			ItemStack *stack = (*inventory)[slot];

			if (stack) {
				Glib::ustring label_text = stack->getTooltip();
				if (stack->count != 1)
					label_text += " \u00d7 " + std::to_string(stack->count);
				if (stack->hasDurability())
					label_text += "\n(" + std::to_string(stack->data.at("durability").at(0).get<Durability>()) + '/' + std::to_string(stack->data.at("durability").at(1).get<Durability>()) + ')';
				auto fixed_ptr = std::make_unique<Gtk::Fixed>();
				auto image_ptr = std::make_unique<Gtk::Image>(inventory->getImage(*game, slot));
				auto label_ptr = std::make_unique<Gtk::Label>(std::to_string(stack->count));
				label_ptr->set_xalign(1.f);
				label_ptr->set_yalign(1.f);
				auto &fixed = *fixed_ptr;
				if (stack->hasDurability()) {
					auto progress_ptr = std::make_unique<Gtk::ProgressBar>();
					progress_ptr->set_fraction(stack->getDurabilityFraction());
					progress_ptr->add_css_class("item-durability");
					progress_ptr->set_size_request(tile_size - InventoryTab::TILE_MAGIC, -1);
					fixed.put(*progress_ptr, 0, 0);
					widgets.push_back(std::move(progress_ptr));
				}
				fixed.put(*image_ptr, 0, 0);
				fixed.put(*label_ptr, 0, 0);
				fixed.set_tooltip_text(label_text);
				widget_ptr = std::move(fixed_ptr);
				image_ptr->set_size_request(tile_size - InventoryTab::TILE_MAGIC, tile_size - InventoryTab::TILE_MAGIC);
				label_ptr->set_size_request(tile_size - InventoryTab::TILE_MAGIC, tile_size - InventoryTab::TILE_MAGIC);
				widgets.push_back(std::move(image_ptr));
				widgets.push_back(std::move(label_ptr));
			} else
				widget_ptr = std::make_unique<Gtk::Label>("");

			if (auto label = dynamic_cast<Gtk::Label *>(widget_ptr.get())) {
				label->set_wrap(true);
				label->set_wrap_mode(Pango::WrapMode::CHAR);
			}

			widget_ptr->set_size_request(tile_size, tile_size);
			widget_ptr->add_css_class("item-slot");

			Gtk::Widget *old_widget = nullptr;
			if (auto iter = widgetsBySlot.find(slot); iter != widgetsBySlot.end()) {
				old_widget = iter->second;
				widgetMap.erase(iter->second);
			}
			widgetsBySlot[slot] = widget_ptr.get();

			auto left_click = Gtk::GestureClick::create();
			left_click->set_button(1);
			left_click->signal_released().connect([this, slot, widget = widget_ptr.get()](int n, double x, double y) {
				const auto mods = clickGestures[widget].first->get_current_event_state();
				leftClick(widget, n, slot, Modifiers{mods}, x, y);
			});

			auto right_click = Gtk::GestureClick::create();
			right_click->set_button(3);
			right_click->signal_pressed().connect([this, slot, widget = widget_ptr.get()](int n, double x, double y) {
				const auto mods = clickGestures[widget].second->get_current_event_state();
				rightClick(widget, n, slot, Modifiers{mods}, x, y);
			});

			widget_ptr->add_controller(left_click);
			widget_ptr->add_controller(right_click);

			clickGestures[widget_ptr.get()] = {std::move(left_click), std::move(right_click)};
			widgetMap[widget_ptr.get()] = slot;

			if (old_widget != nullptr)
				grid.remove(*old_widget);
			grid.attach(*widget_ptr, column, row);
			widgets.push_back(std::move(widget_ptr));
		}

		lastSlotCount = inventory->slotCount;
	}

	void ExternalInventoryModule::leftClick(Gtk::Widget *, int, Slot slot, Modifiers modifiers, double, double) {
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

	void ExternalInventoryModule::rightClick(Gtk::Widget *widget, int, Slot slot, Modifiers, double x, double y) {
		// mainWindow.onBlur();

		if (!inventory->contains(slot))
			return;

		const auto allocation = widget->get_allocation();
		x += allocation.get_x();
		y += allocation.get_y();

		popoverMenu.set_has_arrow(true);
		popoverMenu.set_pointing_to({int(x), int(y), 1, 1});
		popoverMenu.set_menu_model(gmenu);
		lastSlot = slot;
		popoverMenu.popup();
	}
}
