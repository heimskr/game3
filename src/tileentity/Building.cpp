#include "Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "ui/SpriteRenderer.h"

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

	bool Building::onInteractOn(const std::shared_ptr<Player> &player) {
		return onInteractNextTo(player);
	}

	bool Building::onInteractNextTo(const std::shared_ptr<Player> &player) {
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
		return getRealm()->getGame().realms.at(innerRealmID);
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

	void Building::render(SpriteRenderer &sprite_renderer) {
		if (!isVisible())
			return;

		auto realm = getRealm();
		auto &tileset = realm->getTileset();

		if (tileID != tileset.getEmpty()) {
			const auto tilesize = tileset.getTileSize();
			const auto tile_num = tileset[tileID];
			const auto texture = tileset.getTexture(realm->getGame());
			const auto x = (tile_num % (*texture->width / tilesize)) * tilesize;
			const auto y = (tile_num / (*texture->width / tilesize)) * tilesize;
			sprite_renderer(*texture, {
				.x = static_cast<float>(position.column),
				.y = static_cast<float>(position.row),
				.x_offset = x / 2.f,
				.y_offset = y / 2.f,
				.size_x = static_cast<float>(tilesize),
				.size_y = static_cast<float>(tilesize),
			});
		}
	}
}
