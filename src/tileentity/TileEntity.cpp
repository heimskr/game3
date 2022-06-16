#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "tileentity/Chest.h"
#include "tileentity/CraftingStation.h"
#include "tileentity/Sign.h"
#include "tileentity/Stockpile.h"
#include "tileentity/Teleporter.h"
#include "tileentity/TileEntity.h"
#include "tileentity/Tree.h"

namespace Game3 {
	std::shared_ptr<TileEntity> TileEntity::fromJSON(const nlohmann::json &json) {
		const TileEntityID id = json.at("id");
		std::shared_ptr<TileEntity> out;

		switch (id) {
			case TileEntity::BUILDING:
				out = TileEntity::create<Building>();
				break;
			case TileEntity::TELEPORTER:
				out = TileEntity::create<Teleporter>();
				break;
			case TileEntity::SIGN:
				out = TileEntity::create<Sign>();
				break;
			case TileEntity::CHEST:
				out = TileEntity::create<Chest>();
				break;
			case TileEntity::STOCKPILE:
				out = TileEntity::create<Stockpile>();
				break;
			case TileEntity::TREE:
				out = TileEntity::create<Tree>();
				break;
			case TileEntity::CRAFTINGSTATION:
				out = TileEntity::create<CraftingStation>();
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

	std::shared_ptr<Realm> TileEntity::getRealm() const {
		auto out = weakRealm.lock();
		if (!out)
			throw std::runtime_error("Couldn't lock tile entity's realm");
		return out;
	}

	void TileEntity::absorbJSON(const nlohmann::json &json) {
		tileID = json.at("tileID");
		position = json.at("position");
		solid = json.at("solid");
		if (json.contains("extra"))
			extraData = json.at("extra");
	}

	void TileEntity::toJSON(nlohmann::json &json) const {
		json["id"] = getID();
		json["tileID"] = tileID;
		json["position"] = position;
		json["solid"] = solid;
		if (!extraData.empty())
			json["extra"] = extraData;
	}

	void to_json(nlohmann::json &json, const TileEntity &tile_entity) {
		tile_entity.toJSON(json);
	}
}
