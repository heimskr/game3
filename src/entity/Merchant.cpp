#include <iostream>

#include "entity/Merchant.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/tab/MerchantTab.h"

namespace Game3 {
	std::shared_ptr<Merchant> Merchant::create(EntityID id) {
		auto out = std::shared_ptr<Merchant>(new Merchant(id));
		out->init();
		return out;
	}

	std::shared_ptr<Merchant> Merchant::fromJSON(const nlohmann::json &json) {
		auto out = Entity::create<Merchant>(json.at("id"));
		out->absorbJSON(json);
		return out;
	}

	nlohmann::json Merchant::toJSON() const {
		nlohmann::json json;
		to_json(json, *this);
		return json;
	}

	void Merchant::absorbJSON(const nlohmann::json &json) {
		Entity::absorbJSON(json);
		greed = json.at("greed");
		money = json.at("money");
	}

	void Merchant::onInteractNextTo(const std::shared_ptr<Player> &player) {
		auto &window = getRealm()->getGame().canvas.window;
		auto &tab = *window.merchantTab;
		player->queueForMove([player, &tab](const auto &) {
			tab.hide();
			return true;
		});
		tab.show();
		window.delay([this, &tab] {
			tab.setMerchantInventory("Merchant", inventory, greed);
		}, 2);
	}

	void to_json(nlohmann::json &json, const Merchant &merchant) {
		to_json(json, static_cast<const Entity &>(merchant));
		json["money"] = merchant.money;
		json["greed"] = merchant.greed;
	}
}
