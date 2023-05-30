#include "game/ClientGame.h"
#include "game/ServerGame.h"
#include "net/Buffer.h"
#include "net/RemoteClient.h"
#include "packet/TileEntityPacket.h"
#include "realm/Realm.h"
#include "tileentity/TileEntity.h"
#include "tileentity/TileEntityFactory.h"
#include "ui/Canvas.h"

namespace Game3 {
	void TileEntity::destroy() {
		auto realm = getRealm();
		assert(realm);
		realm->removeSafe(shared_from_this());

		if (getSide() == Side::Server) {
			getRealm()->getGame().toServer().tileEntityDestroyed(*this);
		}
	}

	std::shared_ptr<TileEntity> TileEntity::fromJSON(Game &game, const nlohmann::json &json) {
		auto factory = game.registry<TileEntityFactoryRegistry>().at(json.at("id").get<Identifier>());
		assert(factory);
		auto out = (*factory)(game);
		out->absorbJSON(game, json);
		return out;
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
		const auto pos = getPosition();
		auto realm = getRealm();
		if (getSide() == Side::Client)
			return realm->getGame().toClient().canvas.inBounds(pos) && realm->isVisible(pos);
		return realm->isVisible(pos);
	}

	Side TileEntity::getSide() const {
		return getRealm()->getGame().getSide();
	}

	ChunkPosition TileEntity::getChunk() const {
		return getChunkPosition(getPosition());
	}

	void TileEntity::encode(Game &, Buffer &buffer) {
		buffer << tileID;
		buffer << position;
		buffer << solid;
		buffer << extraData.dump();
	}

	void TileEntity::decode(Game &, Buffer &buffer) {
		buffer >> tileID;
		buffer >> position;
		buffer >> solid;
		std::string extra_json;
		buffer >> extra_json;
		extraData = nlohmann::json::parse(extra_json);
	}

	void TileEntity::sendTo(RemoteClient &client) {
		client.send(TileEntityPacket(shared_from_this()));
	}

	void TileEntity::absorbJSON(Game &, const nlohmann::json &json) {
		tileEntityID = json.at("id");
		globalID     = json.at("gid");
		tileID       = json.at("tileID");
		position     = json.at("position");
		solid        = json.at("solid");
		if (auto iter = json.find("extra"); iter != json.end())
			extraData = *iter;
	}

	void TileEntity::toJSON(nlohmann::json &json) const {
		json["id"]       = getID();
		json["gid"]      = globalID;
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
