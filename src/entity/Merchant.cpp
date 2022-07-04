#include <iostream>

#include "entity/Merchant.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/tab/MerchantTab.h"

namespace Game3 {
	Merchant::Merchant(EntityID id__, EntityType type_): Entity(id__, type_) {}

	std::shared_ptr<Merchant> Merchant::create(EntityID id, EntityType type) {
		auto out = std::shared_ptr<Merchant>(new Merchant(id, type));
		out->init();
		return out;
	}

	std::shared_ptr<Merchant> Merchant::fromJSON(const nlohmann::json &json) {
		auto out = Entity::create<Merchant>(json.at("id"));
		out->absorbJSON(json);
		return out;
	}

	void Merchant::toJSON(nlohmann::json &json) const {
		Entity::toJSON(json);
		json["money"] = money;
		json["greed"] = greed;
	}

	void Merchant::absorbJSON(const nlohmann::json &json) {
		Entity::absorbJSON(json);
		greed = json.at("greed");
		money = json.at("money");
	}

	bool Merchant::onInteractNextTo(const std::shared_ptr<Player> &player) {
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
		return true;
	}

	void to_json(nlohmann::json &json, const Merchant &merchant) {
		merchant.toJSON(json);
	}
}
