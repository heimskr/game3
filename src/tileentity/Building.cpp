#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "graphics/SpriteRenderer.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"

namespace Game3 {
	Building::Building(Identifier id_, Position position, RealmID inner_realm_id, Position entrance):
		TileEntity(std::move(id_), ID(), position, true),
		innerRealmID(inner_realm_id),
		entrance(entrance) {}

	void Building::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		json["innerRealmID"] = innerRealmID;
		json["entrance"] = entrance;
	}

	bool Building::onInteractOn(const std::shared_ptr<Player> &player, Modifiers modifiers, const ItemStackPtr &used_item, Hand hand) {
		return onInteractNextTo(player, modifiers, used_item, hand);
	}

	bool Building::onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers, const ItemStackPtr &, Hand) {
		if (getSide() == Side::Client)
			return false;
		teleport(player);
		return true;
	}

	void Building::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		innerRealmID = json.at("innerRealmID");
		entrance = json.at("entrance");
	}

	void Building::teleport(const std::shared_ptr<Entity> &entity) {
		entity->teleport(entrance, getInnerRealm());
	}

	std::shared_ptr<Realm> Building::getInnerRealm() const {
		GamePtr game = getGame();
		return game->getRealm(innerRealmID);
	}

	void Building::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << innerRealmID;
		buffer << entrance;
	}

	void Building::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> innerRealmID;
		buffer >> entrance;
	}
}
