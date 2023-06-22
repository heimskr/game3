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

		if (getSide() == Side::Server)
			getRealm()->getGame().toServer().tileEntityDestroyed(*this);
	}

	std::shared_ptr<TileEntity> TileEntity::fromJSON(Game &game, const nlohmann::json &json) {
		auto factory = game.registry<TileEntityFactoryRegistry>().at(json.at("id").get<Identifier>());
		assert(factory);
		auto out = (*factory)(game);
		out->absorbJSON(game, json);
		return out;
	}

	void TileEntity::init(Game &game) {
		assert(!initialized);
		initialized = true;

		auto lock = game.allAgents.uniqueLock();
		if (game.allAgents.contains(globalID)) {
			if (auto locked = game.allAgents.at(globalID).lock()) {
				ERROR("globalID[" << globalID << "], allAgents<" << game.allAgents.size() << ">, type[this=" << typeid(*this).name() << ", other=" << typeid(*locked).name() << "]");
			} else {
				ERROR("globalID[" << globalID << "], allAgents<" << game.allAgents.size() << ">, type[this=" << typeid(*this).name() << ", other=expired]");
			}
			assert(!game.allAgents.contains(globalID));
		}
		game.allAgents[globalID] = shared_from_this();
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

	void TileEntity::updateNeighbors() const {
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
		buffer << getUpdateCounter();
		buffer << extraData.dump();
	}

	void TileEntity::decode(Game &, Buffer &buffer) {
		buffer >> tileID;
		buffer >> position;
		buffer >> solid;
		setUpdateCounter(buffer.take<UpdateCounter>());
		std::string extra_json;
		buffer >> extra_json;
		extraData = nlohmann::json::parse(extra_json);
	}

	void TileEntity::sendTo(RemoteClient &client, UpdateCounter threshold) {
		if (threshold == 0 || getUpdateCounter() < threshold) {
			client.send(TileEntityPacket(shared_from_this()));
			onSend(client.getPlayer());
		}
	}

	void TileEntity::absorbJSON(Game &, const nlohmann::json &json) {
		tileEntityID = json.at("id");
		globalID     = json.at("gid");
		tileID       = json.at("tileID");
		position     = json.at("position");
		solid        = json.at("solid");
		if (auto iter = json.find("extra"); iter != json.end())
			extraData = *iter;
		increaseUpdateCounter();
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
