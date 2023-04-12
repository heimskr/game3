#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/TileEntity.h"
#include "tileentity/TileEntityFactory.h"
#include "ui/Canvas.h"

namespace Game3 {
	std::shared_ptr<TileEntity> TileEntity::fromJSON(Game &game, const nlohmann::json &json) {
		auto factory = game.registry<TileEntityFactoryRegistry>().at(json.at("id").get<Identifier>());
		assert(factory);
		auto out = (*factory)(game);
		out->absorbJSON(game, json);
		return out;
	}

	void TileEntity::init(Game &game, std::default_random_engine &) {
		init(game);
	}

	void TileEntity::init(Game &game) {
		std::default_random_engine fakeRNG;
		init(game, fakeRNG);
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

	void TileEntity::absorbJSON(Game &, const nlohmann::json &json) {
		tileEntityID = json.at("id");
		tileID       = json.at("tileID");
		position     = json.at("position");
		solid        = json.at("solid");
		if (json.contains("extra"))
			extraData = json.at("extra");
	}

	void TileEntity::toJSON(nlohmann::json &json) const {
		json["id"]       = getID();
		json["tileID"]   = tileID;
		json["position"] = position;
		json["solid"]    = solid;
		if (!extraData.empty())
			json["extra"] = extraData;
	}

	void to_json(nlohmann::json &json, const TileEntity &tile_entity) {
		tile_entity.toJSON(json);
	}
}
