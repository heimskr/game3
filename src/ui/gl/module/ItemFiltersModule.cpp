#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "packet/SetItemFiltersPacket.h"
#include "pipes/ItemFilter.h"
#include "tileentity/Pipe.h"
#include "ui/gl/module/ItemFiltersModule.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Button.h"
#include "ui/gl/widget/Checkbox.h"
#include "ui/gl/widget/Grid.h"
#include "ui/gl/widget/Icon.h"
#include "ui/gl/widget/IconButton.h"
#include "ui/gl/widget/ItemSlot.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/Constants.h"
#include "util/Util.h"

namespace Game3 {
	ItemFiltersModule::ItemFiltersModule(UIContext &ui, const std::shared_ptr<ClientGame> &game, const std::any &argument):
		ItemFiltersModule(ui, game, std::any_cast<DirectedPlace>(argument)) {}

	ItemFiltersModule::ItemFiltersModule(UIContext &ui, const std::shared_ptr<ClientGame> &game, const DirectedPlace &place):
		Module(ui), weakGame(game), place(place) {}

	void ItemFiltersModule::init() {
		vbox = std::make_shared<Box>(ui, scale, Orientation::Vertical, 5, 0, Color{});
		vbox->insertAtEnd(shared_from_this());

		topHBox = std::make_shared<Box>(ui, scale, Orientation::Horizontal, 2, 0, Color{});
		topHBox->insertAtEnd(vbox);

		auto copy_button = std::make_shared<Button>(ui, scale);
		copy_button->setText("Copy");
		copy_button->setAlignment(Alignment::Middle);
		copy_button->setFixedHeight(12 * scale);
		copy_button->setOnClick([this](Widget &, int button, int, int) {
			if (button == 1)
				copy();
			return true;
		});
		copy_button->insertAtEnd(topHBox);

		dropSlot = std::make_shared<ItemSlot>(ui);
		dropSlot->setHorizontalExpand(true);
		dropSlot->setAlignment(Alignment::Middle);
		dropSlot->insertAtEnd(topHBox);
		dropSlot->onDrop.connect([this](ItemSlot &, const WidgetPtr &widget) {
			onDrop(widget);
		});

		auto paste_button = std::make_shared<Button>(ui, scale);
		paste_button->setText("Paste");
		paste_button->setAlignment(Alignment::Middle);
		paste_button->setFixedHeight(12 * scale);
		paste_button->setOnClick([this](Widget &, int button, int, int) {
			if (button == 1)
				paste();
			return true;
		});
		paste_button->insertAtEnd(topHBox);

		secondHBox = std::make_shared<Box>(ui, scale, Orientation::Horizontal, 2, 0, Color{});
		secondHBox->insertAtEnd(vbox);

		whitelistCheckbox = std::make_shared<Checkbox>(ui, scale);
		whitelistCheckbox->setFixedSize(8 * scale);
		whitelistCheckbox->insertAtEnd(secondHBox);
		if (filter)
			whitelistCheckbox->setChecked(filter->isAllowMode());

		auto whitelist_label = std::make_shared<Label>(ui, scale);
		whitelist_label->setText("Whitelist");
		whitelist_label->setHorizontalExpand(true);
		whitelist_label->insertAtEnd(secondHBox);
		whitelist_label->setOnClick([this](Widget &, int button, int, int) -> bool {
			if (button != 1)
				return false;
			const bool whitelist = !whitelistCheckbox->getChecked();
			whitelistCheckbox->setChecked(whitelist);
			setWhitelist(whitelist);
			return true;
		});

		strictCheckbox = std::make_shared<Checkbox>(ui, scale);
		strictCheckbox->setFixedSize(8 * scale);
		strictCheckbox->insertAtEnd(secondHBox);
		if (filter)
			strictCheckbox->setChecked(filter->isStrict());

		auto strict_label = std::make_shared<Label>(ui, scale);
		strict_label->setText("Strict");
		strict_label->insertAtEnd(secondHBox);
		strict_label->setOnClick([this](Widget &, int button, int, int) -> bool {
			if (button != 1)
				return false;
			const bool strict = !strictCheckbox->getChecked();
			strictCheckbox->setChecked(strict);
			setStrict(strict);
			return true;
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
			ClientGamePtr game = weakGame.lock();
			assert(game);
			game->getPlayer()->copyItemFilter(*filter);
		}
	}

	void ItemFiltersModule::paste() {
		ClientGamePtr game = weakGame.lock();
		assert(game);

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
		if (!filter_to_use)
			filter_to_use = filter;

		if (!filter_to_use)
			return;

		if (!pipe) {
			WARN("Pipe is missing in ItemFiltersModule::upload");
			return;
		}

		ClientGamePtr game = weakGame.lock();

		if (!game)
			throw std::runtime_error("Game is missing in ItemFiltersModule::upload");

		game->getPlayer()->send(SetItemFiltersPacket(pipe->getGID(), place.direction, *filter_to_use));
	}

	bool ItemFiltersModule::setFilter() {
		if (!setPipe())
			return false;

		auto &filter_ref = pipe->itemFilters[place.direction];

		if (!filter_ref)
			filter_ref = std::make_shared<ItemFilter>();

		if (filter != filter_ref)
			filter = filter_ref;

		return true;
	}

	bool ItemFiltersModule::saveFilter() {
		if (!setPipe())
			return false;

		pipe->itemFilters[place.direction] = filter;
		return true;
	}

	bool ItemFiltersModule::setPipe() {
		if (!pipe)
			pipe = std::dynamic_pointer_cast<Pipe>(place.getTileEntity());
		return pipe != nullptr;
	}

	void ItemFiltersModule::onDrop(const WidgetPtr &source) {
		auto item_slot = std::dynamic_pointer_cast<ItemSlot>(source);
		if (!item_slot)
			return;

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
		for (const auto &[id, set]: configs)
			for (const ItemFilter::Config &config: set)
				addHBox(id, config);
	}

	void ItemFiltersModule::addHBox(const Identifier &id, const ItemFilter::Config &config) {
		ClientGamePtr game = weakGame.lock();
		assert(game);
		ItemStackPtr stack = ItemStack::create(game, id, 1, config.data);
		auto hbox = std::make_shared<Box>(ui, scale, Orientation::Horizontal, 2, 0, Color{});
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
			auto spacer = std::make_shared<Label>(ui, scale);
			spacer->setHorizontalExpand(true);
			hbox->append(spacer);
		}
		hbox->append(button);
		vbox->append(hbox);
	}

	std::shared_ptr<Icon> ItemFiltersModule::makeImage(ItemStack &stack) {
		ClientGamePtr game = weakGame.lock();
		assert(game);

		auto image = std::make_shared<Icon>(ui, scale);
		image->setFixedSize(8 * scale);
		image->setIconTexture(stack.getTexture(*game));
		return image;
	}

	std::shared_ptr<Label> ItemFiltersModule::makeLabel(const ItemStack &stack) {
		auto label = std::make_shared<Label>(ui, scale);
		label->setText(stack.getTooltip());
		return label;
	}

	std::shared_ptr<Button> ItemFiltersModule::makeComparator(const Identifier &id, const ItemFilter::Config &config) {
		auto button = std::make_shared<Button>(ui, scale);
		button->setExpand(false, false);

		if (config.comparator == ItemFilter::Comparator::Less)
			button->setText("<");
		else if (config.comparator == ItemFilter::Comparator::Greater)
			button->setText(">");
		else
			button->setText("~");

		button->setOnClick([this, id = id, config = config](Widget &, int button, int, int) {
			if (button != 1)
				return false;
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
			return true;
		});

		return button;
	}

	std::shared_ptr<TextInput> ItemFiltersModule::makeThreshold(const Identifier &id, const ItemFilter::Config &config) {
		if (config.comparator == ItemFilter::Comparator::None)
			return {};

		auto threshold = std::make_shared<TextInput>(ui, scale);
		threshold->setHorizontalExpand(true);
		threshold->setFixedHeight(10 * scale);
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
		auto button = std::make_shared<IconButton>(ui, scale);
		button->setIconTexture(cacheTexture("resources/gui/minus.png"));
		button->setOnClick([this, stack = stack->copy()](Widget &, int button, int, int) {
			if (button != 1)
				return false;
			setFilter();
			if (filter) {
				filter->removeItem(stack);
				populate();
				upload();
			}
			return true;
		});
		return button;
	}
}
