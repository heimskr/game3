#include "Log.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "entity/ClientPlayer.h"
#include "item/Item.h"
#include "packet/SetItemFiltersPacket.h"
#include "tileentity/Pipe.h"
#include "types/DirectedPlace.h"
#include "ui/gtk/UITypes.h"
#include "ui/module/ItemFilterModule.h"

namespace Game3 {
	ItemFilterModule::ItemFilterModule(std::shared_ptr<ClientGame> game_, const std::any &argument):
	game(std::move(game_)),
	place(std::any_cast<DirectedPlace>(argument)) {
		auto target = Gtk::DropTarget::create(Glib::Value<DragSource>::value_type(), Gdk::DragAction::MOVE);
		target->signal_drop().connect([this](const Glib::ValueBase &base, double, double) {
			if (!filter || base.gobj()->g_type != Glib::Value<DragSource>::value_type())
				return false;

			const DragSource source = static_cast<const Glib::Value<DragSource> &>(base).get();

			ItemStack *stack = nullptr;
			{
				auto lock = source.inventory->sharedLock();
				stack = (*source.inventory)[source.slot];
			}

			if (stack) {
				setFilter();
				filter->addItem(*stack);
				populate();
				upload();
			}

			return true;
		}, false);

		fixed.add_controller(target);
		fixed.set_size_request(68, 68);
		fixed.set_halign(Gtk::Align::CENTER);
		fixed.add_css_class("item-slot");

		setFilter();

		auto mode_click = Gtk::GestureClick::create();
		mode_click->signal_released().connect([this](int, double, double) {
			modeSwitch.set_active(!modeSwitch.get_active());
		});
		if (filter)
			modeSwitch.set_active(filter->isAllowMode());
		modeSwitch.signal_state_set().connect([this](bool value) {
			setMode(value);
			return false;
		}, false);
		modeSwitch.set_margin_start(10);
		modeLabel.set_margin_start(10);
		modeLabel.add_controller(mode_click);
		modeHbox.set_margin_top(10);
		modeHbox.append(modeSwitch);
		modeHbox.append(modeLabel);

		auto strict_click = Gtk::GestureClick::create();
		strict_click->signal_released().connect([this](int, double, double) {
			strictSwitch.set_active(!strictSwitch.get_active());
		});
		if (filter)
			strictSwitch.set_active(filter->isStrict());
		strictSwitch.signal_state_set().connect([this](bool value) {
			setStrict(value);
			return false;
		}, false);
		strictSwitch.set_margin_start(10);
		strictLabel.set_margin_start(10);
		strictLabel.add_controller(strict_click);
		strictHbox.set_margin_top(10);
		strictHbox.append(strictSwitch);
		strictHbox.append(strictLabel);

		modeHbox.set_hexpand(true);
		strictHbox.set_hexpand(true);
		switchesHbox.append(modeHbox);
		switchesHbox.append(strictHbox);

		populate();
	}

	Gtk::Widget & ItemFilterModule::getWidget() {
		return vbox;
	}

	void ItemFilterModule::update() {
		populate();
	}

	void ItemFilterModule::reset() {
		populate();
	}

	std::optional<Buffer> ItemFilterModule::handleMessage(const std::shared_ptr<Agent> &, const std::string &, std::any &) {
		return std::nullopt;
	}

	void ItemFilterModule::setMode(bool allow) {
		setFilter();
		if (filter) {
			filter->setAllowMode(allow);
			upload();
		}
	}

	void ItemFilterModule::setStrict(bool strict) {
		setFilter();
		if (filter) {
			filter->setStrict(strict);
			upload();
		}
	}

	void ItemFilterModule::upload() {
		if (!filter)
			return;

		if (!pipe) {
			WARN("Pipe is missing in ItemFilterModule::upload");
			return;
		}

		if (!game)
			throw std::runtime_error("Game is missing in ItemFilterModule::upload");

		game->player->send(SetItemFiltersPacket(pipe->getGID(), place.direction, *filter));
	}

	void ItemFilterModule::setFilter() {
		if (!pipe)
			pipe = std::dynamic_pointer_cast<Pipe>(place.getTileEntity());

		if (!pipe)
			return;

		auto &filter_ref = pipe->itemFilters[place.direction];
		if (!filter_ref)
			filter_ref = std::make_shared<ItemFilter>();
		if (filter != filter_ref) {
			INFO(filter << " → " << filter_ref);
			filter = filter_ref;
		}
	}

	void ItemFilterModule::populate() {
		removeChildren(vbox);
		widgets.clear();

		vbox.append(fixed);
		vbox.append(switchesHbox);

		setFilter();

		std::shared_lock<DefaultMutex> configs_lock;
		auto &configs = filter->getConfigs(configs_lock);
		for (const auto &[id, set]: configs)
			for (const auto &config: set)
				addHbox(id, config);
	}

	void ItemFilterModule::addHbox(const Identifier &id, const ItemFilter::Config &config) {
		ItemStack stack(*game, id, 1, config.data);
		auto hbox = std::make_unique<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
		auto image = makeImage(stack);
		auto label = makeLabel(stack);
		auto comparator = makeComparator(id, config);
		auto button = makeButton(std::move(stack));
		hbox->set_margin_top(10);
		hbox->append(*image);
		hbox->append(*label);
		hbox->append(*comparator);
		auto spacer = std::make_unique<Gtk::Label>();
		spacer->set_hexpand(true);
		hbox->append(*spacer);
		hbox->append(*button);
		vbox.append(*hbox);
		widgets.push_back(std::move(button));
		widgets.push_back(std::move(spacer));
		widgets.push_back(std::move(comparator));
		widgets.push_back(std::move(label));
		widgets.push_back(std::move(image));
		widgets.push_back(std::move(hbox));
	}

	std::unique_ptr<Gtk::Image> ItemFilterModule::makeImage(ItemStack &stack) {
		auto image = std::make_unique<Gtk::Image>(stack.getImage(*game));
		image->set_margin(10);
		image->set_margin_top(6);
		image->set_size_request(32, 32);
		return image;
	}

	std::unique_ptr<Gtk::Label> ItemFilterModule::makeLabel(const ItemStack &stack) {
		auto label = std::make_unique<Gtk::Label>(stack.getTooltip());
		label->set_halign(Gtk::Align::START);
		label->set_margin_end(10);
		return label;
	}

	std::unique_ptr<Gtk::Button> ItemFilterModule::makeComparator(const Identifier &id, const ItemFilter::Config &config) {
		auto button = std::make_unique<Gtk::Button>();

		if (config.comparator == ItemFilter::Comparator::Less)
			button->set_label("<");
		else if (config.comparator == ItemFilter::Comparator::Greater)
			button->set_label(">");
		else
			button->set_label("~");

		button->signal_clicked().connect([this, id = id, config = config, button = button.get()] {
			setFilter();
			{
				std::unique_lock<DefaultMutex> lock;
				auto &configs = filter->getConfigs(lock);
				auto &set = configs[id];
				ItemFilter::Config new_config = std::move(set.extract(config).value());

				if (new_config.comparator == ItemFilter::Comparator::Less) {
					new_config.comparator = ItemFilter::Comparator::None;
				} else if (new_config.comparator == ItemFilter::Comparator::Greater) {
					new_config.comparator = ItemFilter::Comparator::Less;
				} else {
					new_config.comparator = ItemFilter::Comparator::Greater;
				}

				set.insert(std::move(new_config));
			}
			populate();
			upload();
		});

		return button;
	}

	std::unique_ptr<Gtk::Button> ItemFilterModule::makeButton(ItemStack stack) {
		auto button = std::make_unique<Gtk::Button>();
		button->set_icon_name("list-remove-symbolic");
		button->set_vexpand(false);
		button->set_has_frame(false);
		button->set_margin_end(10);
		button->signal_clicked().connect([this, stack = std::move(stack)] {
			setFilter();
			if (filter) {
				filter->removeItem(stack);
				populate();
				upload();
			}
		});
		return button;
	}
}
