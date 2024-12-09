#include <iostream>

#include "entity/Merchant.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "net/Buffer.h"
#include "realm/Realm.h"
#include "ui/Window.h"

namespace Game3 {
	Merchant::Merchant(EntityType type_):
		Entity(std::move(type_)) {}

	std::shared_ptr<Merchant> Merchant::create(const std::shared_ptr<Game> &, EntityType type) {
		return Entity::create<Merchant>(std::move(type));
	}

	std::shared_ptr<Merchant> Merchant::fromJSON(const GamePtr &game, const boost::json::value &json) {
		auto out = Entity::create<Merchant>(boost::json::value_to<Identifier>(json.at("id")));
		out->absorbJSON(game, json);
		return out;
	}

	void Merchant::toJSON(boost::json::value &json) const {
		Entity::toJSON(json);
		json.as_object()["greed"] = greed;
	}

	void Merchant::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		Entity::absorbJSON(game, json);
		greed = json.at("greed").as_double();
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

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const Merchant &merchant) {
		merchant.toJSON(json);
	}
}
