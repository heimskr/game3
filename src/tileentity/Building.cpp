#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "graphics/SpriteRenderer.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"

namespace Game3 {
	Building::Building(Identifier id_, Position position_, RealmID inner_realm_id, Position entrance_):
		TileEntity(std::move(id_), ID(), std::move(position_), true),
		innerRealmID(inner_realm_id),
		entrance(std::move(entrance_)) {}

	void Building::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["innerRealmID"] = innerRealmID;
		json["entrance"] = entrance;
	}

	bool Building::onInteractOn(const std::shared_ptr<Player> &player, Modifiers modifiers, ItemStack *used_item) {
		return onInteractNextTo(player, modifiers, used_item);
	}

	bool Building::onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers, ItemStack *) {
		if (getSide() == Side::Client)
			return false;
		teleport(player);
		return true;
	}

	void Building::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		innerRealmID = json.at("innerRealmID");
		entrance = json.at("entrance");
	}

	void Building::teleport(const std::shared_ptr<Entity> &entity) {
		entity->teleport(entrance, getInnerRealm());
	}

	std::shared_ptr<Realm> Building::getInnerRealm() const {
		return getRealm()->getGame().getRealm(innerRealmID);
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
