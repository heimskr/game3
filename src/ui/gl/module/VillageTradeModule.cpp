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
#include "ui/gl/UIContext.h"
#include "util/Util.h"

#include <cassert>

namespace Game3 {
	VillageTradeModule::VillageTradeModule(UIContext &ui, const std::shared_ptr<ClientGame> &game, const std::any &argument):
		Module(ui, game),
		village(std::any_cast<VillagePtr>(argument)) {}

	void VillageTradeModule::init() {
		assert(village);

		vbox = std::make_shared<Box>(ui, scale, Orientation::Vertical, 2, 0, Color{});
		vbox->insertAtEnd(shared_from_this());

		villageName = std::make_shared<Label>(ui, scale);
		villageName->setText(village->getName());
		villageName->setHorizontalAlignment(Alignment::Center);
		villageName->setHorizontalExpand(true);
		vbox->append(villageName);

		laborLabel = std::make_shared<Label>(ui, scale);
		laborLabel->setHorizontalAlignment(Alignment::Center);
		laborLabel->setHorizontalExpand(true);
		vbox->append(laborLabel);

		sellRow = std::make_shared<Box>(ui, scale, Orientation::Vertical, 2, 0, Color{});
		sellRow->setExpand(true, false);

		totalPriceLabel = std::make_shared<Label>(ui, scale);
		unitPriceLabel = std::make_shared<Label>(ui, scale);

		sellLabelBox = std::make_shared<Box>(ui, scale, Orientation::Vertical, 2, 0, Color{});
		sellLabelBox->setExpand(true, true);
		sellLabelBox->append(totalPriceLabel);
		sellLabelBox->append(unitPriceLabel);

		sellSlot = std::make_shared<ItemSlot>(ui);

		sellCount = std::make_shared<TextInput>(ui, scale);
		sellCount->setText("0");

		sellButton = std::make_shared<Button>(ui, scale);
		sellButton->setText("Sell");
		sellButton->setVerticalAlignment(Alignment::Center);
		sellButton->setOnClick([this](Widget &, int button, int, int) {
			if (button == 1)
				sell();
			return true;
		});

		sellRow->append(sellSlot);
		sellRow->append(sellCount);
		sellRow->append(sellButton);
		sellRow->append(sellLabelBox);
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
		rows.clear();
		update();
	}

	void VillageTradeModule::update() {
		villageName->setText(village->getName());
		laborLabel->setText(std::format("Available labor: {:.2f}", village->getLabor()));
		if (ItemStackPtr stack = sellSlot->getStack())
			updateSell(stack);
		populate();
	}

	std::optional<Buffer> VillageTradeModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "VillageUpdate") {

			VillagePtr updated_village = std::any_cast<VillagePtr>(data);
			if (updated_village == village)
				update();

		}

		return {};
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
			totalPriceLabel->setText(std::format("Total: {}", *sell_price));
			unitPriceLabel->setText(std::format("Unit: {:.2f}", *sell_price / old_value));
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
		game->getPlayer()->send(DoVillageTradePacket(village->getID(), stack->getID(), sell_count, true));
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
				auto row = std::make_shared<VillageTradeRow>(ui, scale, game, village->getID(), *game->getItem(resource), amount);
				row->init();
				vbox->append(row);
				rows[resource] = std::move(row);
			}
		}
	}

	VillageTradeRow::VillageTradeRow(UIContext &ui, float scale, const ClientGamePtr &game, VillageID village_id, const Item &item, double amount_):
		Box(ui, scale, Orientation::Horizontal, 2, 0, Color{}),
		weakGame(game),
		villageID(village_id),
		resource(item.identifier),
		// itemSlot(game, -1, nullptr),
		basePrice(item.basePrice),
		amount(amount_) {}

	void VillageTradeRow::init() {
		ClientGamePtr game = weakGame.lock();
		assert(game);

		itemSlot = std::make_shared<ItemSlot>(ui);
		quantityLabel = std::make_shared<Label>(ui, scale);
		transferAmount = std::make_shared<TextInput>(ui, scale);
		buyButton = std::make_shared<Button>(ui, scale);

		buyButton->setText("Buy");
		buyButton->setFixedHeight(10 * scale);
		buyButton->setVerticalAlignment(Alignment::Center);
		itemSlot->setStack(ItemStack::create(game, resource, ItemCount(-1)));
		updateLabel();
		updateTooltips(1);
		quantityLabel->setFixedWidth(64);
		quantityLabel->setVerticalAlignment(Alignment::Center);
		transferAmount->setText("0");
		transferAmount->setFixedWidth(16 * scale);
		transferAmount->setVerticalAlignment(Alignment::Center);

		transferAmount->onChange.connect([this](TextInput &, const UString &) {
			try {
				updateTooltips(getCount());
			} catch (const std::invalid_argument &) {}
		});

		buyButton->setOnClick([this](Widget &, int button, int, int) {
			if (button == 1) {
				if (ClientGamePtr game = weakGame.lock()) {
					const std::string &amount_text = transferAmount->getText().raw();
					ItemCount amount{};

					try {
						amount = parseNumber<ItemCount>(amount_text);
					} catch (const std::invalid_argument &) {
						return true;
					}

					buy(game, ItemCount());
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
		const ItemCount old_count = getCount();
		// transferAmount.set_adjustment(Gtk::Adjustment::create(1.0, 1.0, std::min(amount, 999.)));
		transferAmount->setText(std::to_string(std::min({static_cast<double>(old_count), amount, 999.})));
	}

	void VillageTradeRow::updateLabel() {
		quantityLabel->setText(std::format("x {:.2f}", amount));
	}

	void VillageTradeRow::updateTooltips(ItemCount count) {
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
		game->getPlayer()->send(DoVillageTradePacket(villageID, resource, amount, false));
	}

	void VillageTradeRow::sell(const ClientGamePtr &game, ItemCount amount) {
		game->getPlayer()->send(DoVillageTradePacket(villageID, resource, amount, true));
	}
}
