#include "game/Building.h"
#include "game/Realm.h"
#include "game/TileEntity.h"

namespace Game3 {
	std::shared_ptr<TileEntity> TileEntity::fromJSON(const nlohmann::json &json) {
		const TileEntityID id = json.at("id");
		std::shared_ptr<TileEntity> out;

		switch (id) {
			case TileEntity::BUILDING:
				out = TileEntity::create<Building>();
				break;
			default:
				throw std::invalid_argument("Unrecognized TileEntity ID: " + std::to_string(id));
		}

		out->absorbJSON(json);
		return out;
	}

	void TileEntity::remove() {
		auto shared = shared_from_this();
	}

	void TileEntity::setRealm(const std::shared_ptr<Realm> &realm) {
		realmID = realm->id;
		weakRealm = realm;
	}

	void TileEntity::absorbJSON(const nlohmann::json &json) {
		tileID = json.at("tileID");
		position = json.at("position");
		solid = json.at("solid");
	}

	void TileEntity::toJSON(nlohmann::json &json) const {
		json["id"] = getID();
		json["tileID"] = tileID;
		json["position"] = position;
		json["solid"] = solid;
	}

	void to_json(nlohmann::json &json, const TileEntity &tile_entity) {
		tile_entity.toJSON(json);
	}
}
