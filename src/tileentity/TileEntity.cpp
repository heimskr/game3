#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "tileentity/Chest.h"
#include "tileentity/CraftingStation.h"
#include "tileentity/Ghost.h"
#include "tileentity/ItemSpawner.h"
#include "tileentity/OreDeposit.h"
#include "tileentity/Sign.h"
#include "tileentity/Stockpile.h"
#include "tileentity/Teleporter.h"
#include "tileentity/TileEntity.h"
#include "tileentity/Tree.h"
#include "ui/Canvas.h"

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
			case TileEntity::CRAFTING_STATION:
				out = TileEntity::create<CraftingStation>();
				break;
			case TileEntity::ORE_DEPOSIT:
				out = TileEntity::create<OreDeposit>(json.at("type"));
				break;
			case TileEntity::GHOST:
				out = TileEntity::create<Ghost>();
				break;
			case TileEntity::ITEM_SPAWNER:
				out = TileEntity::create<ItemSpawner>();
				break;
			default:
				throw std::invalid_argument("Unrecognized TileEntity ID: " + std::to_string(id));
		}

		out->absorbJSON(json);
		return out;
	}

	void TileEntity::init(std::default_random_engine &) {
		init();
	}

	void TileEntity::init() {
		std::default_random_engine fakeRNG;
		init(fakeRNG);
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

	void TileEntity::updateNeighbors() {
		getRealm()->updateNeighbors(position);
	}

	bool TileEntity::isVisible() const {
		return getRealm()->getGame().canvas.inBounds(getPosition());
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
