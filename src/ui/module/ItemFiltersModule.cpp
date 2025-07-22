#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "graphics/RendererContext.h"
#include "packet/SetItemFiltersPacket.h"
#include "pipes/ItemFilter.h"
#include "tileentity/Pipe.h"
#include "ui/module/ItemFiltersModule.h"
#include "ui/widget/Box.h"
#include "ui/widget/Button.h"
#include "ui/widget/Checkbox.h"
#include "ui/widget/Grid.h"
#include "ui/widget/Icon.h"
#include "ui/widget/IconButton.h"
#include "ui/widget/ItemSlot.h"
#include "ui/widget/Label.h"
#include "ui/widget/Spacer.h"
#include "ui/widget/TextInput.h"
#include "ui/Constants.h"
#include "ui/UIContext.h"
#include "util/Util.h"

namespace Game3 {
	ItemFiltersModule::ItemFiltersModule(UIContext &ui, float selfScale, const std::shared_ptr<ClientGame> &game, const std::any &argument):
		ItemFiltersModule(ui, selfScale, game, std::any_cast<DirectedPlace>(argument)) {}

	ItemFiltersModule::ItemFiltersModule(UIContext &ui, float selfScale, const std::shared_ptr<ClientGame> &game, const DirectedPlace &place):
		Module(ui, selfScale, game), place(place) {}

	void ItemFiltersModule::init() {
		vbox = make<Box>(ui, selfScale, Orientation::Vertical, 5, 0, Color{});
		vbox->insertAtEnd(shared_from_this());
		vbox->setName("ItemFiltersModule::vbox");

		topHBox = make<Box>(ui, selfScale, Orientation::Horizontal, 2, 0, Color{});
		topHBox->setName("ItemFiltersModule::topHBox");
		vbox->append(topHBox);

		auto copy_button = make<Button>(ui, selfScale);
		copy_button->setText("Copy");
		copy_button->setAlignment(Alignment::Center);
		copy_button->setFixedHeight(12 * selfScale);
		copy_button->setOnClick([this](Widget &) {
			copy();
		});
		copy_button->insertAtEnd(topHBox);

		dropSlot = make<ItemSlot>(ui);
		dropSlot->setHorizontalExpand(true);
		dropSlot->setAlignment(Alignment::Center);
		dropSlot->insertAtEnd(topHBox);
		dropSlot->onDrop.connect([this](ItemSlot &, const WidgetPtr &widget) {
			onDrop(widget);
		});

		auto paste_button = make<Button>(ui, selfScale);
		paste_button->setText("Paste");
		paste_button->setAlignment(Alignment::Center);
		paste_button->setFixedHeight(12 * selfScale);
		paste_button->setOnClick([this](Widget &) {
			paste();
		});
		paste_button->insertAtEnd(topHBox);

		secondHBox = make<Box>(ui, selfScale, Orientation::Horizontal, 2, 0, Color{});
		secondHBox->setName("SecondHbox");
		vbox->append(secondHBox);

		whitelistCheckbox = make<Checkbox>(ui, selfScale);
		whitelistCheckbox->setFixedSize(8 * selfScale);
		whitelistCheckbox->insertAtEnd(secondHBox);
		if (filter)
			whitelistCheckbox->setChecked(filter->isAllowMode());

		auto whitelist_label = make<Label>(ui, selfScale);
		whitelist_label->setText("Whitelist");
		whitelist_label->setHorizontalExpand(true);
		whitelist_label->insertAtEnd(secondHBox);
		whitelist_label->setOnClick([this](Widget &) {
			const bool whitelist = !whitelistCheckbox->getChecked();
			whitelistCheckbox->setChecked(whitelist);
			setWhitelist(whitelist);
		});

		strictCheckbox = make<Checkbox>(ui, selfScale);
		strictCheckbox->setFixedSize(8 * selfScale);
		strictCheckbox->insertAtEnd(secondHBox);
		if (filter)
			strictCheckbox->setChecked(filter->isStrict());

		auto strict_label = make<Label>(ui, selfScale);
		strict_label->setText("Strict");
		strict_label->insertAtEnd(secondHBox);
		strict_label->setOnClick([this](Widget &) {
			const bool strict = !strictCheckbox->getChecked();
			strictCheckbox->setChecked(strict);
			setStrict(strict);
		});

		populate();
	}

	void ItemFiltersModule::render(const RendererContext &renderers, float x, float y, float width, float height) {
		Widget::render(renderers, x, y, width, height);
		firstChild->render(renderers, x, y, width, height);
	}

	SizeRequestMode ItemFiltersModule::getRequestMode() const {
		return firstChild->getRequestMode();
	}

	void ItemFiltersModule::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		return firstChild->measure(renderers, orientation, for_width, for_height, minimum, natural);
	}

	void ItemFiltersModule::copy() {;
		if (filter) {
			ClientGamePtr game = getGame();
			game->getPlayer()->copyItemFilter(*filter);
		}
	}

	void ItemFiltersModule::paste() {
		ClientGamePtr game = getGame();

		if (auto pasted = game->getPlayer()->pasteItemFilter()) {
			auto shared_filter = std::make_shared<ItemFilter>(*pasted);
			filter = shared_filter;
			whitelistCheckbox->setChecked(filter->isAllowMode());
			strictCheckbox->setChecked(filter->isStrict());
			saveFilter();
			populate(shared_filter);
			upload(shared_filter);
		}
	}

	void ItemFiltersModule::setWhitelist(bool whitelist) {
		setFilter();
		if (filter) {
			filter->setAllowMode(whitelist);
			upload();
		}
	}

	void ItemFiltersModule::setStrict(bool strict) {
		setFilter();
		if (filter) {
			filter->setStrict(strict);
			upload();
		}
	}

	void ItemFiltersModule::upload(ItemFilterPtr filter_to_use) {
		if (!filter_to_use) {
			filter_to_use = filter;
			if (!filter_to_use) {
				return;
			}
		}

		if (!pipe) {
			WARN("Pipe is missing in ItemFiltersModule::upload");
			return;
		}

		getGame()->getPlayer()->send(make<SetItemFiltersPacket>(pipe->getGID(), place.direction, *filter_to_use));
	}

	bool ItemFiltersModule::setFilter() {
		if (!setPipe()) {
			return false;
		}

		auto &filter_ref = pipe->itemFilters[place.direction];

		if (!filter_ref) {
			filter_ref = std::make_shared<ItemFilter>();
		}

		if (filter != filter_ref) {
			filter = filter_ref;
		}

		return true;
	}

	bool ItemFiltersModule::saveFilter() {
		if (!setPipe()) {
			return false;
		}

		pipe->itemFilters[place.direction] = filter;
		return true;
	}

	bool ItemFiltersModule::setPipe() {
		if (!pipe) {
			pipe = std::dynamic_pointer_cast<Pipe>(place.getTileEntity());
		}

		return pipe != nullptr;
	}

	void ItemFiltersModule::onDrop(const WidgetPtr &source) {
		auto item_slot = std::dynamic_pointer_cast<ItemSlot>(source);
		if (!item_slot) {
			return;
		}

		if (ItemStackPtr stack = item_slot->getStack()) {
			setFilter();
			filter->addItem(stack);
			populate();
			upload();
		}
	}

	void ItemFiltersModule::populate(ItemFilterPtr filter_to_use) {
		vbox->clearChildren();

		vbox->append(topHBox);
		vbox->append(secondHBox);

		if (!filter_to_use) {
			setFilter();
			filter_to_use = filter;
		}

		whitelistCheckbox->setChecked(filter_to_use->isAllowMode());
		strictCheckbox->setChecked(filter_to_use->isStrict());

		std::shared_lock<DefaultMutex> configs_lock;
		auto &configs = filter_to_use->getConfigs(configs_lock);
		int i = 0;
		for (const auto &[id, set]: configs) {
			for (const ItemFilter::Config &config: set) {
				addHBox(id, config)->setName(std::format("ItemFiltersModule::hbox{}", i++));
			}
		}

		if (WidgetPtr parent = getParent()) {
			parent->onChildrenUpdated();
		}
	}

	std::shared_ptr<Box> ItemFiltersModule::addHBox(const Identifier &id, const ItemFilter::Config &config) {
		ClientGamePtr game = getGame();
		ItemStackPtr stack = ItemStack::create(game, id, 1, config.data);
		auto hbox = make<Box>(ui, selfScale, Orientation::Horizontal, 2, 0, Color{});
		auto image = makeImage(*stack);
		auto label = makeLabel(*stack);
		auto comparator = makeComparator(id, config);
		auto threshold = makeThreshold(id, config);
		auto button = makeButton(stack);
		hbox->append(image);
		hbox->append(label);
		hbox->append(comparator);
		if (threshold) {
			hbox->append(threshold);
		} else {
			hbox->append(make<Spacer>(ui, Orientation::Horizontal));
		}
		hbox->append(button);
		vbox->append(hbox);
		return hbox;
	}

	std::shared_ptr<Icon> ItemFiltersModule::makeImage(ItemStack &stack) {
		ClientGamePtr game = getGame();
		auto image = make<Icon>(ui, selfScale);
		image->setFixedSize(8 * selfScale);
		image->setIconTexture(stack.getTexture(*game));
		return image;
	}

	std::shared_ptr<Label> ItemFiltersModule::makeLabel(const ItemStack &stack) {
		auto label = make<Label>(ui, selfScale);
		label->setText(stack.getTooltip());
		return label;
	}

	std::shared_ptr<Button> ItemFiltersModule::makeComparator(const Identifier &id, const ItemFilter::Config &config) {
		auto button = make<Button>(ui, selfScale);
		button->setExpand(false, false);

		if (config.comparator == ItemFilter::Comparator::Less) {
			button->setText("<");
		} else if (config.comparator == ItemFilter::Comparator::Greater) {
			button->setText(">");
		} else {
			button->setText("~");
		}

		button->setOnClick([this, id = id, config = config](Widget &) {
			setFilter();
			{
				std::unique_lock<DefaultMutex> lock;
				auto &configs = filter->getConfigs(lock);
				auto &set = configs[id];
				auto extracted = set.extract(config);
				ItemFilter::Config &new_config = extracted.value();

				if (new_config.comparator == ItemFilter::Comparator::Less) {
					new_config.comparator = ItemFilter::Comparator::None;
				} else if (new_config.comparator == ItemFilter::Comparator::Greater) {
					new_config.comparator = ItemFilter::Comparator::Less;
				} else {
					new_config.comparator = ItemFilter::Comparator::Greater;
				}

				set.insert(std::move(extracted));
			}
			populate();
			upload();
		});

		return button;
	}

	std::shared_ptr<TextInput> ItemFiltersModule::makeThreshold(const Identifier &id, const ItemFilter::Config &config) {
		if (config.comparator == ItemFilter::Comparator::None) {
			return {};
		}

		auto threshold = make<TextInput>(ui, selfScale);
		threshold->setHorizontalExpand(true);
		threshold->setFixedHeight(10 * selfScale);
		threshold->setText(std::to_string(config.count));
		threshold->onChange.connect([this, id = id, config = config, threshold = threshold.get()](TextInput &, const UString &text) mutable {
			setFilter();
			ItemCount count{};
			try {
				count = parseNumber<ItemCount>(text.raw());
			} catch (const std::invalid_argument &) {
				return;
			}
			{
				std::unique_lock<DefaultMutex> lock;
				auto &configs = filter->getConfigs(lock);
				auto &set = configs[id];
				config = std::move(set.extract(config).value());
				config.count = count;
				set.insert(config);
			}
			upload();
		});

		return threshold;
	}

	std::shared_ptr<Button> ItemFiltersModule::makeButton(const ItemStackPtr &stack) {
		auto button = make<IconButton>(ui, selfScale);
		button->setIconTexture(cacheTexture("resources/gui/minus.png"));
		button->setOnClick([this, stack = stack->copy()](Widget &) {
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
