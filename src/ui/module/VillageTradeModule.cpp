#include "algorithm/Stonks.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "game/Village.h"
#include "item/Item.h"
#include "packet/DoVillageTradePacket.h"
#include "ui/gtk/DragSource.h"
#include "ui/gtk/Util.h"
#include "ui/module/VillageTradeModule.h"
#include "ui/tab/InventoryTab.h"
#include "ui/MainWindow.h"
#include "util/Util.h"

#include <format>

namespace Game3 {
	VillageTradeModule::VillageTradeModule(std::shared_ptr<ClientGame> game_, const std::any &argument):
	game(std::move(game_)),
	village(std::any_cast<VillagePtr>(argument)),
	sellSlot(game, -1, {}, {}) {
		vbox.set_hexpand(true);
		vbox.set_vexpand(false);
		villageName.set_xalign(0.5);
		villageName.set_hexpand(true);
		villageName.set_margin_top(10);
		villageName.set_margin_bottom(5);
		laborLabel.set_xalign(0.5);
		laborLabel.set_hexpand(true);
		laborLabel.set_margin_bottom(5);

		sellRow.set_hexpand(true);
		sellRow.set_vexpand(false);
		sellRow.set_margin_start(10);
		sellSlot.set_hexpand(false);
		sellSlot.set_halign(Gtk::Align::START);
		sellCount.set_hexpand(false);
		sellCount.set_halign(Gtk::Align::CENTER);
		sellCount.set_valign(Gtk::Align::CENTER);
		sellCount.set_margin_start(10);
		sellCount.set_margin_end(10);
		sellCount.set_adjustment(Gtk::Adjustment::create(0.0, 0.0, 0.0));
		sellButton.set_hexpand(false);
		sellButton.set_halign(Gtk::Align::START);
		sellButton.set_valign(Gtk::Align::CENTER);
		sellButton.add_css_class("buy-sell-button");
		sellLabelBox.set_hexpand(true);
		sellLabelBox.set_vexpand(true);
		sellLabelBox.set_margin_start(5);
		totalPriceLabel.set_halign(Gtk::Align::START);
		unitPriceLabel.set_halign(Gtk::Align::START);
		sellLabelBox.set_valign(Gtk::Align::CENTER);
		sellLabelBox.append(totalPriceLabel);
		sellLabelBox.append(unitPriceLabel);
		sellRow.append(sellSlot);
		sellRow.append(sellCount);
		sellRow.append(sellButton);
		sellRow.append(sellLabelBox);
		sellButton.signal_clicked().connect(sigc::mem_fun(*this, &VillageTradeModule::sell));

		sellSlot.onDrop = [this](const ItemStackPtr &stack) {
			if (stack)
				setSellStack(stack);
			else
				WARN_("No stack in sellSlot.onDrop");
			return true;
		};

		auto right_click = Gtk::GestureClick::create();
		right_click->set_button(3);
		right_click->signal_released().connect([this](int, double, double) {
			sellSlot.reset();
			hideSell();
		});
		sellSlot.add_controller(right_click);

		dropTarget = Gtk::DropTarget::create(Glib::Value<DragSource>::value_type(), Gdk::DragAction::MOVE);

		dropTarget->signal_enter().connect([this](double, double) {
			game->getWindow().queue([this] {
				showSell();
			});
			return Gdk::DragAction::MOVE;
		}, false);

		dropTarget->signal_leave().connect([this] {
			game->getWindow().queue([this] {
				if (sellSlot.empty())
					hideSell();
			});
		}, true);

		vbox.add_controller(dropTarget);
	}

	Gtk::Widget & VillageTradeModule::getWidget() {
		return vbox;
	}

	void VillageTradeModule::reset() {
		removeChildren(vbox);
		vbox.append(villageName);
		vbox.append(laborLabel);
		rows.clear();
		update();
	}

	void VillageTradeModule::update() {
		villageName.set_text(village->getName());
		laborLabel.set_text(std::format("Available labor: {:.2f}", village->getLabor()));
		if (const ItemStackPtr &stack = sellSlot.getStack())
			updateSell(stack);
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

		sellSlot.setStack(stack);
		const double max(stack->count);
		sellCount.set_adjustment(Gtk::Adjustment::create(max, 1.0, max));
		sellCount.set_value(max);
		updateSell(stack);
		return true;
	}

	void VillageTradeModule::updateSell(const ItemStackPtr &stack) {
		if (!village) {
			WARN_("No village in VillageTradeModule::setSellStack");
			return;
		}

		const double old_value = sellCount.get_value();
		const double max(game->getPlayer()->getInventory(0)->count(stack));
		sellCount.set_adjustment(Gtk::Adjustment::create(max, 1.0, max));
		sellCount.set_value(old_value);

		std::optional<double> amount = village->getResourceAmount(stack->getID());

		if (std::optional<MoneyCount> sell_price = totalSellPrice(amount.value_or(0.0), -1, stack->item->basePrice, ItemCount(sellCount.get_value()), village->getGreed())) {
			sellButton.set_tooltip_text(std::format("Price: {}", *sell_price));
			totalPriceLabel.set_text(std::format("Total: {}", *sell_price));
			unitPriceLabel.set_text(std::format("Unit: {:.2f}", *sell_price / sellCount.get_value()));
		} else {
			sellButton.set_tooltip_text("Village lacks funds!");
		}
	}

	void VillageTradeModule::sell() {
		if (!village)
			return;

		ItemStackPtr &stack = sellSlot.getStack();
		if (!stack)
			return;

		const ItemCount sell_count(sellCount.get_value());

		ItemCount inventory_count = game->getPlayer()->getInventory(0)->count(stack);

		if (inventory_count <= sell_count) {
			sellSlot.reset();
			hideSell();
		} else {
			// If the inventory count is less than the displayed stack count, update the displayed stack count.
			inventory_count -= sell_count;
			if (inventory_count < stack->count) {
				stack->count = inventory_count;
				sellSlot.setStack(stack);
			}
		}

		game->getPlayer()->send(DoVillageTradePacket(village->getID(), stack->getID(), sell_count, true));
	}

	void VillageTradeModule::showSell() {
		if (sellRowShown)
			return;

		sellRowShown = true;
		vbox.insert_child_after(sellRow, laborLabel);
	}

	void VillageTradeModule::hideSell() {
		if (!sellRowShown)
			return;

		sellRowShown = false;
		vbox.remove(sellRow);
	}

	void VillageTradeModule::populate() {
		assert(village);

		const auto &resources = village->getResources();
		const auto lock = resources.sharedLock();

		for (auto iter = rows.begin(); iter != rows.end();) {
			const auto &[resource, row] = *iter;
			if (auto resource_iter = resources.find(resource); resource_iter == resources.end() || resource_iter->second < 1.0) {
				vbox.remove(*row);
				rows.erase(iter++);
			} else {
				++iter;
			}
		}

		for (const auto &[resource, amount]: resources) {
			if (auto iter = rows.find(resource); iter != rows.end()) {
				iter->second->setAmount(amount);
				iter->second->updateLabel();
			} else if (1.0 <= amount) {
				auto row = std::make_unique<Row>(game, village->getID(), *game->getItem(resource), amount, village->getGreed());
				vbox.append(*row);
				rows[resource] = std::move(row);
			}
		}
	}

	VillageTradeModule::Row::Row(const ClientGamePtr &game, VillageID village_id, const Item &item, double amount_, double greed_):
	Gtk::Box(Gtk::Orientation::HORIZONTAL),
	villageID(village_id),
	resource(item.identifier),
	itemSlot(game, -1, nullptr),
	basePrice(item.basePrice),
	amount(amount_),
	greed(greed_) {
		itemSlot.setStack(ItemStack::create(game, resource, ItemCount(-1)));
		updateLabel();
		updateTooltips(1);
		itemSlot.set_hexpand(false);
		itemSlot.set_margin_start(3);
		quantityLabel.set_size_request(64, -1);
		quantityLabel.set_xalign(0.0);
		quantityLabel.set_margin_start(5);
		transferAmount.set_valign(Gtk::Align::CENTER);
		transferAmount.set_adjustment(Gtk::Adjustment::create(1.0, 1.0, std::min(amount, 999.0)));
		transferAmount.set_digits(0);
		buyButton.set_valign(Gtk::Align::CENTER);
		buyButton.set_margin_start(5);
		buyButton.add_css_class("buy-sell-button");
		set_margin_top(5);
		set_halign(Gtk::Align::CENTER);

		transferAmount.signal_changed().connect([this] {
			updateTooltips(getCount());
		});

		buyButton.signal_clicked().connect([this, weak_game = std::weak_ptr(game)]() {
			if (ClientGamePtr game = weak_game.lock())
				buy(game, ItemCount(transferAmount.get_value()));
		});

		append(itemSlot);
		append(quantityLabel);
		append(transferAmount);
		append(buyButton);
	}

	void VillageTradeModule::Row::setAmount(double amount_) {
		if (amount == amount_)
			return;
		amount = amount_;
		const ItemCount old_count = getCount();
		transferAmount.set_adjustment(Gtk::Adjustment::create(1.0, 1.0, std::min(amount, 999.0)));
		transferAmount.set_value(std::min({double(old_count), amount, 999.0}));
	}

	void VillageTradeModule::Row::updateLabel() {
		quantityLabel.set_text(std::format("× {:.2f}", amount));
	}

	void VillageTradeModule::Row::updateTooltips(ItemCount count) {
		if (std::optional<MoneyCount> buy_price = totalBuyPrice(ItemCount(amount), -1, basePrice, count))
			buyButton.set_tooltip_text(std::format("Price: {}", *buy_price));
		else
			buyButton.set_tooltip_text("Village doesn't have that many!");
	}

	ItemCount VillageTradeModule::Row::getCount() const {
		return ItemCount(transferAmount.get_value());
	}

	void VillageTradeModule::Row::buy(const ClientGamePtr &game, ItemCount amount) {
		game->getPlayer()->send(DoVillageTradePacket(villageID, resource, amount, false));
	}

	void VillageTradeModule::Row::sell(const ClientGamePtr &game, ItemCount amount) {
		game->getPlayer()->send(DoVillageTradePacket(villageID, resource, amount, true));
	}
}
