#include "Log.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "entity/ClientPlayer.h"
#include "item/Item.h"
#include "packet/SetItemFiltersPacket.h"
#include "tileentity/Pipe.h"
#include "types/DirectedPlace.h"
#include "ui/gtk/DragSource.h"
#include "ui/gtk/Util.h"
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

			auto source_lock = source.inventory->sharedLock();
			ItemStackPtr stack = (*source.inventory)[source.slot];

			if (stack) {
				setFilter();
				filter->addItem(stack);
				populate();
				upload();
			}

			return true;
		}, false);

		fixed.add_controller(target);
		fixed.set_size_request(68, 68);
		fixed.set_halign(Gtk::Align::CENTER);
		fixed.add_css_class("item-slot");

		copyButton.set_hexpand(true);
		copyButton.set_valign(Gtk::Align::CENTER);
		copyButton.set_halign(Gtk::Align::START);
		copyButton.set_margin_start(10);
		copyButton.set_icon_name("edit-copy-symbolic");

		pasteButton.set_hexpand(true);
		pasteButton.set_valign(Gtk::Align::CENTER);
		pasteButton.set_halign(Gtk::Align::END);
		pasteButton.set_margin_end(10);
		pasteButton.set_icon_name("edit-paste-symbolic");

		topBox.append(copyButton);
		topBox.append(fixed);
		topBox.append(pasteButton);

		copyButton.signal_clicked().connect([this] {
			INFO_("Clicked Copy");
			if (filter) {
				game->getPlayer()->copyItemFilter(*filter);
				pasteButton.set_sensitive(true);
			}
		});

		pasteButton.signal_clicked().connect([this] {
			if (auto pasted = game->getPlayer()->pasteItemFilter()) {
				auto shared_filter = std::make_shared<ItemFilter>(*pasted);
				filter = shared_filter;
				modeSwitch.set_active(filter->isAllowMode());
				strictSwitch.set_active(filter->isStrict());
				saveFilter();
				populate(shared_filter);
				upload(shared_filter);
			}
		});

		pasteButton.set_sensitive(game->getPlayer()->pasteItemFilter().has_value());

		setFilter();

		if (filter)
			modeSwitch.set_active(filter->isAllowMode());

		modeSwitch.signal_state_set().connect([this](bool value) {
			setMode(value);
			return false;
		}, false);
		modeSwitch.set_margin_start(10);
		modeLabel.set_margin_start(10);
		modeLabel.add_controller(createClick([this] {
			modeSwitch.set_active(!modeSwitch.get_active());
		}));
		modeHbox.set_margin_top(10);
		modeHbox.append(modeSwitch);
		modeHbox.append(modeLabel);

		if (filter)
			strictSwitch.set_active(filter->isStrict());

		strictSwitch.signal_state_set().connect([this](bool value) {
			setStrict(value);
			return false;
		}, false);
		strictSwitch.set_margin_start(10);
		strictLabel.set_margin_start(10);
		strictLabel.add_controller(createClick([this] {
			strictSwitch.set_active(!strictSwitch.get_active());
		}));
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

	bool ItemFilterModule::handleShiftClick(std::shared_ptr<Inventory> source_inventory, Slot source_slot) {
		if (ItemStackPtr stack = (*source_inventory)[source_slot]) {
			setFilter();
			filter->addItem(stack);
			populate();
			upload();
		}

		return true;
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

	void ItemFilterModule::upload(ItemFilterPtr filter_to_use) {
		if (!filter_to_use)
			filter_to_use = filter;

		if (!filter_to_use)
			return;

		if (!pipe) {
			WARN_("Pipe is missing in ItemFilterModule::upload");
			return;
		}

		if (!game)
			throw std::runtime_error("Game is missing in ItemFilterModule::upload");

		game->getPlayer()->send(SetItemFiltersPacket(pipe->getGID(), place.direction, *filter_to_use));
	}

	bool ItemFilterModule::setFilter() {
		if (!pipe)
			pipe = std::dynamic_pointer_cast<Pipe>(place.getTileEntity());

		if (!pipe)
			return false;

		auto &filter_ref = pipe->itemFilters[place.direction];

		if (!filter_ref)
			filter_ref = std::make_shared<ItemFilter>();

		if (filter != filter_ref)
			filter = filter_ref;

		return true;
	}

	bool ItemFilterModule::saveFilter() {
		if (!pipe)
			pipe = std::dynamic_pointer_cast<Pipe>(place.getTileEntity());

		if (!pipe)
			return false;

		pipe->itemFilters[place.direction] = filter;
		return true;
	}

	void ItemFilterModule::populate(ItemFilterPtr filter_to_use) {
		removeChildren(vbox);
		widgets.clear();

		vbox.append(topBox);
		vbox.append(switchesHbox);

		if (!filter_to_use) {
			setFilter();
			filter_to_use = filter;
		}

		std::shared_lock<DefaultMutex> configs_lock;
		auto &configs = filter_to_use->getConfigs(configs_lock);
		for (const auto &[id, set]: configs)
			for (const auto &config: set)
				addHbox(id, config);
	}

	void ItemFilterModule::addHbox(const Identifier &id, const ItemFilter::Config &config) {
		ItemStackPtr stack = ItemStack::create(game, id, 1, config.data);
		auto hbox = std::make_unique<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
		auto image = makeImage(*stack);
		auto label = makeLabel(*stack);
		auto comparator = makeComparator(id, config);
		auto threshold = makeThreshold(id, config);
		auto button = makeButton(stack);
		hbox->set_margin_top(10);
		hbox->append(*image);
		hbox->append(*label);
		hbox->append(*comparator);
		if (threshold) {
			hbox->append(*threshold);
			widgets.push_back(std::move(threshold));
		} else {
			auto spacer = std::make_unique<Gtk::Label>();
			spacer->set_hexpand(true);
			hbox->append(*spacer);
			widgets.push_back(std::move(spacer));
		}
		hbox->append(*button);
		vbox.append(*hbox);
		widgets.push_back(std::move(button));
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
		button->set_expand(false);
		button->set_has_frame(false);

		if (config.comparator == ItemFilter::Comparator::Less)
			button->set_label("<");
		else if (config.comparator == ItemFilter::Comparator::Greater)
			button->set_label(">");
		else
			button->set_label("~");

		button->add_css_class("comparator-button");

		button->signal_clicked().connect([this, id = id, config = config] {
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

	std::unique_ptr<Gtk::SpinButton> ItemFilterModule::makeThreshold(const Identifier &id, const ItemFilter::Config &config) {
		if (config.comparator == ItemFilter::Comparator::None)
			return {};

		auto spin = std::make_unique<Gtk::SpinButton>();
		spin->set_adjustment(Gtk::Adjustment::create(0., 0., 1e9));
		spin->set_digits(0);
		spin->set_value(config.count);
		spin->set_hexpand(true);

		spin->signal_value_changed().connect([this, id = id, config = config, spin = spin.get()]() mutable {
			setFilter();
			{
				std::unique_lock<DefaultMutex> lock;
				auto &configs = filter->getConfigs(lock);
				auto &set = configs[id];
				config = std::move(set.extract(config).value());
				config.count = spin->get_value();
				set.insert(config);
			}
			upload();
		});

		return spin;
	}

	std::unique_ptr<Gtk::Button> ItemFilterModule::makeButton(const ItemStackPtr &stack) {
		auto button = std::make_unique<Gtk::Button>();
		button->set_icon_name("list-remove-symbolic");
		button->set_expand(false);
		button->set_has_frame(false);
		button->set_margin_end(10);
		button->signal_clicked().connect([this, stack = stack->copy()] {
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
