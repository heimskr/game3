#include "algorithm/Stonks.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "game/Village.h"
#include "packet/DoVillageTradePacket.h"
#include "ui/gl/module/VillageTradeModule.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Button.h"
#include "ui/gl/widget/ItemSlot.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "util/Util.h"

#include <cassert>

namespace Game3 {
	VillageTradeModule::VillageTradeModule(UIContext &ui, float selfScale, const std::shared_ptr<ClientGame> &game, const std::any &argument):
		Module(ui, selfScale, game),
		village(std::any_cast<VillagePtr>(argument)) {}

	void VillageTradeModule::init() {
		assert(village);

		vbox = std::make_shared<Box>(ui, selfScale, Orientation::Vertical, 2, 0, Color{});
		vbox->insertAtEnd(shared_from_this());

		villageName = std::make_shared<Label>(ui, selfScale);
		villageName->setText(village->getName());
		villageName->setHorizontalAlignment(Alignment::Center);
		villageName->setHorizontalExpand(true);
		vbox->append(villageName);

		laborLabel = std::make_shared<Label>(ui, selfScale);
		laborLabel->setHorizontalAlignment(Alignment::Center);
		laborLabel->setHorizontalExpand(true);
		vbox->append(laborLabel);

		sellRow = std::make_shared<Box>(ui, selfScale, Orientation::Horizontal, 2, 0, Color{});
		sellRow->setExpand(true, false);

		totalPriceLabel = std::make_shared<Label>(ui, selfScale);
		unitPriceLabel = std::make_shared<Label>(ui, selfScale);

		sellLabelBox = std::make_shared<Box>(ui, selfScale, Orientation::Vertical, 2, 0, Color{});
		sellLabelBox->setExpand(true, false);
		sellLabelBox->append(totalPriceLabel);
		sellLabelBox->append(unitPriceLabel);

		sellSlot = std::make_shared<ItemSlot>(ui);
		sellSlot->onDrop.connect([this](ItemSlot &, const WidgetPtr &dropped_widget) {
			auto dropped = std::dynamic_pointer_cast<ItemSlot>(dropped_widget);
			if (!dropped)
				return;

			if (ItemStackPtr stack = dropped->getStack()) {
				setSellStack(std::move(stack));
			} else {
				WARN("No stack in sellSlot->onDrop");
			}
		});

		sellCount = std::make_shared<TextInput>(ui, selfScale);
		sellCount->setText("0");
		sellCount->setFixedWidth(16 * selfScale);
		sellCount->setVerticalAlignment(Alignment::Center);

		sellButton = std::make_shared<Button>(ui, selfScale);
		sellButton->setText("Sell");
		sellButton->setVerticalAlignment(Alignment::Center);
		sellButton->setFixedHeight(10 * selfScale);
		sellButton->setOnClick([this](Widget &, int button, int, int) {
			if (button == LEFT_BUTTON)
				sell();
			return true;
		});

		sellRow->append(sellSlot);
		sellRow->append(sellCount);
		sellRow->append(sellButton);
		sellRow->append(sellLabelBox);

		sellRow->bestowAttribute("sellRow");
		vbox->append(sellRow);
		sellRowShown = true;

		showSell();
	}

	void VillageTradeModule::render(const RendererContext &renderers, float x, float y, float width, float height) {
		Module::render(renderers, x, y, width, height);
		firstChild->render(renderers, x, y, width, height);
	}

	SizeRequestMode VillageTradeModule::getRequestMode() const {
		return firstChild->getRequestMode();
	}

	void VillageTradeModule::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		return firstChild->measure(renderers, orientation, for_width, for_height, minimum, natural);
	}

	void VillageTradeModule::reset() {
		vbox->clearChildren();
		vbox->append(villageName);
		vbox->append(laborLabel);
		if (sellRowShown)
			vbox->append(sellRow);
		rows.clear();
		update();
	}

	void VillageTradeModule::update() {
		if (!village)
			return;
		villageName->setText(village->getName());
		laborLabel->setText(std::format("Available labor: {:.2f}", village->getLabor()));
		if (sellSlot) {
			if (ItemStackPtr stack = sellSlot->getStack()) {
				updateSell(stack);
			}
		}
		populate();
	}

	std::optional<Buffer> VillageTradeModule::handleMessage(const std::shared_ptr<Agent> &, const std::string &name, std::any &data) {
		if (name == "VillageUpdate") {

			VillagePtr updated_village = std::any_cast<VillagePtr>(data);
			if (updated_village == village)
				update();

		}

		return {};
	}

	bool VillageTradeModule::handleShiftClick(std::shared_ptr<Inventory> source_inventory, Slot slot) {
		if (ItemStackPtr stack = (*source_inventory)[slot]) {
			if (setSellStack(stack)) {
				showSell();
				return true;
			}
		}

		return false;
	}

	bool VillageTradeModule::setSellStack(ItemStackPtr stack) {
		if (!isSellable(stack))
			return false;

		sellSlot->setStack(stack);
		// const double max(stack->count);
		// sellCount.set_adjustment(Gtk::Adjustment::create(max, 1.0, max));
		sellCount->setText(std::to_string(stack->count));
		updateSell(stack);
		return true;
	}

	void VillageTradeModule::updateSell(const ItemStackPtr &stack) {
		if (!village) {
			WARN("No village in VillageTradeModule::updateSell");
			return;
		}

		ClientGamePtr game = getGame();

		double old_value{};

		try {
			old_value = parseNumber<double>(sellCount->getText().raw());
		} catch (const std::invalid_argument &) {}

		// const double max(game->getPlayer()->getInventory(0)->count(stack));
		// sellCount.set_adjustment(Gtk::Adjustment::create(max, 1.0, max));
		// sellCount->setText(std::to_string(old_value));

		std::optional<double> amount = village->getResourceAmount(stack->getID());

		if (std::optional<MoneyCount> sell_price = totalSellPrice(amount.value_or(0.0), -1, stack->item->basePrice, static_cast<ItemCount>(old_value), village->getGreed())) {
			// sellButton.setTooltipText(std::format("Price: {}", *sell_price));
			if (old_value == 0) {
				totalPriceLabel->setText({});
				unitPriceLabel->setText({});
			} else {
				totalPriceLabel->setText(std::format("Total: {}", *sell_price));
				unitPriceLabel->setText(std::format("Unit: {:.2f}", *sell_price / old_value));
			}
		} else {
			// sellButton.setTooltipText("Village lacks funds!");
		}
	}

	void VillageTradeModule::sell() {
		assert(village);

		ClientGamePtr game = getGame();

		ItemStackPtr stack = sellSlot->getStack();
		if (!stack)
			return;

		ItemCount sell_count;

		try {
			sell_count = parseNumber<ItemCount>(sellCount->getText().raw());
		} catch (const std::invalid_argument &) {
			return;
		}

		ItemCount inventory_count = game->getPlayer()->getInventory(0)->count(stack);

		if (inventory_count <= sell_count) {
			sellSlot.reset();
			hideSell();
		} else {
			// If the inventory count is less than the displayed stack count, update the displayed stack count.
			inventory_count -= sell_count;
			if (inventory_count < stack->count) {
				stack->count = inventory_count;
				sellSlot->setStack(stack);
			}
		}

		assert(stack);
		game->getPlayer()->send(make<DoVillageTradePacket>(village->getID(), stack->getID(), sell_count, true));
	}

	void VillageTradeModule::showSell() {
		if (sellRowShown)
			return;

		sellRowShown = true;
		sellRow->insertAfter(vbox, laborLabel);
	}

	void VillageTradeModule::hideSell() {
		if (!sellRowShown)
			return;

		sellRowShown = false;
		vbox->remove(sellRow);
	}

	void VillageTradeModule::populate() {
		assert(village);

		const Lockable<Resources> &resources = village->getResources();
		const auto lock = resources.sharedLock();

		for (auto iter = rows.begin(); iter != rows.end();) {
			const auto &[resource, row] = *iter;
			if (auto resource_iter = resources.find(resource); resource_iter == resources.end() || resource_iter->second < 1.0) {
				vbox->remove(row);
				rows.erase(iter++);
			} else {
				++iter;
			}
		}

		ClientGamePtr game = getGame();

		for (const auto &[resource, amount]: resources) {
			if (auto iter = rows.find(resource); iter != rows.end()) {
				iter->second->setAmount(amount);
				iter->second->updateLabel();
			} else if (1.0 <= amount) {
				auto row = std::make_shared<VillageTradeRow>(ui, selfScale, game, village->getID(), *game->getItem(resource), amount);
				row->init();
				vbox->append(row);
				rows[resource] = std::move(row);
			}
		}
	}

	VillageTradeRow::VillageTradeRow(UIContext &ui, float selfScale, const ClientGamePtr &game, VillageID village_id, const Item &item, double amount_):
		Box(ui, selfScale, Orientation::Horizontal, 2, 0, Color{}),
		weakGame(game),
		villageID(village_id),
		resource(item.identifier),
		// basePrice(item.basePrice),
		amount(amount_) {}

	void VillageTradeRow::init() {
		ClientGamePtr game = weakGame.lock();
		assert(game);

		itemSlot = std::make_shared<ItemSlot>(ui);
		quantityLabel = std::make_shared<Label>(ui, selfScale);
		transferAmount = std::make_shared<TextInput>(ui, selfScale);
		buyButton = std::make_shared<Button>(ui, selfScale);

		buyButton->setText("Buy");
		buyButton->setFixedHeight(10 * selfScale);
		buyButton->setVerticalAlignment(Alignment::Center);
		itemSlot->setStack(ItemStack::create(game, resource, ItemCount(-1)));
		updateLabel();
		updateTooltips(1);
		quantityLabel->setFixedWidth(64);
		quantityLabel->setVerticalAlignment(Alignment::Center);
		transferAmount->setText("0");
		transferAmount->setFixedWidth(16 * selfScale);
		transferAmount->setVerticalAlignment(Alignment::Center);

		transferAmount->onChange.connect([this](TextInput &, const UString &) {
			try {
				updateTooltips(getCount());
			} catch (const std::invalid_argument &) {}
		});

		buyButton->setOnClick([this](Widget &, int button, int, int) {
			if (button == LEFT_BUTTON) {
				if (ClientGamePtr game = weakGame.lock()) {
					const std::string &amount_text = transferAmount->getText().raw();
					ItemCount amount{};

					try {
						amount = parseNumber<ItemCount>(amount_text);
					} catch (const std::invalid_argument &) {
						WARN("Couldn't parse \"{}\"", amount_text);
						return true;
					}

					buy(game, amount);
				}
			}

			return true;
		});

		append(itemSlot);
		append(quantityLabel);
		append(transferAmount);
		append(buyButton);
	}

	void VillageTradeRow::setAmount(double new_amount) {
		if (amount == new_amount)
			return;
		amount = new_amount;
		// const ItemCount old_count = getCount();
		// transferAmount.set_adjustment(Gtk::Adjustment::create(1.0, 1.0, std::min(amount, 999.)));
		// transferAmount->setText(std::to_string(static_cast<ItemCount>(std::min({static_cast<double>(old_count), amount, 999.}))));
	}

	void VillageTradeRow::updateLabel() {
		quantityLabel->setText(std::format("x {:.2f}", amount));
	}

	void VillageTradeRow::updateTooltips(ItemCount count) {
		(void) count;
		// if (std::optional<MoneyCount> buy_price = totalBuyPrice(static_cast<ItemCount>(amount), -1, basePrice, count)) {
		// 	buyButton->setTooltip(std::format("Price: {}", *buy_price));
		// } else {
		// 	buyButton->setTooltip("Village doesn't have that many!");
		// }
	}

	ItemCount VillageTradeRow::getCount() const {
		return parseNumber<ItemCount>(transferAmount->getText().raw());
	}

	void VillageTradeRow::buy(const ClientGamePtr &game, ItemCount amount) {
		game->getPlayer()->send(make<DoVillageTradePacket>(villageID, resource, amount, false));
	}

	void VillageTradeRow::sell(const ClientGamePtr &game, ItemCount amount) {
		game->getPlayer()->send(make<DoVillageTradePacket>(villageID, resource, amount, true));
	}
}
