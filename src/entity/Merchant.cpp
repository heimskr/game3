#include <iostream>

#include "entity/Merchant.h"
#include "game/ClientGame.h"
#include "net/Buffer.h"
#include "realm/Realm.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/tab/MerchantTab.h"

namespace Game3 {
	Merchant::Merchant(EntityType type_):
		Entity(std::move(type_)) {}

	std::shared_ptr<Merchant> Merchant::create(Game &game, EntityType type) {
		auto out = Entity::create<Merchant>(std::move(type));
		out->init(game);
		return out;
	}

	std::shared_ptr<Merchant> Merchant::fromJSON(Game &game, const nlohmann::json &json) {
		auto out = Entity::create<Merchant>(json.at("id"));
		out->absorbJSON(game, json);
		return out;
	}

	void Merchant::toJSON(nlohmann::json &json) const {
		Entity::toJSON(json);
		json["money"] = money;
		json["greed"] = greed;
	}

	void Merchant::absorbJSON(Game &game, const nlohmann::json &json) {
		Entity::absorbJSON(game, json);
		greed = json.at("greed");
		money = json.at("money");
	}

	bool Merchant::onInteractNextTo(const std::shared_ptr<Player> &player) {
		if (getSide() == Side::Client) {
			auto &window = getRealm()->getGame().toClient().canvas.window;
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
		return true;
	}

	void Merchant::encode(Buffer &buffer) {
		buffer << money;
		buffer << greed;
	}

	void Merchant::decode(Buffer &buffer) {
		buffer >> money;
		buffer >> greed;
	}

	void to_json(nlohmann::json &json, const Merchant &merchant) {
		merchant.toJSON(json);
	}
}
