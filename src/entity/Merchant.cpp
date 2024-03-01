#include <iostream>

#include "entity/Merchant.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "net/Buffer.h"
#include "realm/Realm.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"

namespace Game3 {
	Merchant::Merchant(EntityType type_):
		Entity(std::move(type_)) {}

	std::shared_ptr<Merchant> Merchant::create(const std::shared_ptr<Game> &, EntityType type) {
		return Entity::create<Merchant>(std::move(type));
	}

	std::shared_ptr<Merchant> Merchant::fromJSON(const GamePtr &game, const nlohmann::json &json) {
		auto out = Entity::create<Merchant>(json.at("id"));
		out->absorbJSON(game, json);
		return out;
	}

	void Merchant::toJSON(nlohmann::json &json) const {
		Entity::toJSON(json);
		json["greed"] = greed;
	}

	void Merchant::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
		Entity::absorbJSON(game, json);
		greed = json.at("greed");
	}

	bool Merchant::onInteractNextTo(const PlayerPtr &, Modifiers, const ItemStackPtr &, Hand) {
		// if (getSide() == Side::Client) {
		// 	auto &window = getRealm()->getGame().toClient().canvas.window;
		// 	auto &tab = *window.merchantTab;
		// 	player->queueForMove([&tab](const auto &) {
		// 		tab.hide();
		// 		return true;
		// 	});
		// 	tab.show();
		// 	window.delay([this, &tab] {
		// 		tab.setMerchantInventory("Merchant", std::dynamic_pointer_cast<ClientInventory>(inventory), greed);
		// 	}, 2);
		// }
		return true;
	}

	void Merchant::encode(Buffer &buffer) {
		buffer << greed;
	}

	void Merchant::decode(Buffer &buffer) {
		buffer >> greed;
	}

	void to_json(nlohmann::json &json, const Merchant &merchant) {
		merchant.toJSON(json);
	}
}
