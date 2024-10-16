#include "entity/ClientPlayer.h"
#include "graphics/Tileset.h"
#include "game/ClientGame.h"
#include "game/ServerGame.h"
#include "net/Buffer.h"
#include "net/RemoteClient.h"
#include "packet/TileEntityPacket.h"
#include "realm/Realm.h"
#include "tileentity/TileEntity.h"
#include "tileentity/TileEntityFactory.h"
#include "ui/Window.h"
#include "util/Cast.h"

namespace Game3 {
	void TileEntity::destroy() {
		RealmPtr realm = getRealm();
		assert(realm);
		TileEntityPtr self = getSelf();
		realm->removeSafe(self);

		if (getSide() == Side::Server) {
			ServerGame &game = realm->getGame()->toServer();
			game.getDatabase().deleteTileEntity(self);
			game.tileEntityDestroyed(*this);
		}
	}

	std::shared_ptr<TileEntity> TileEntity::fromJSON(const GamePtr &game, const nlohmann::json &json) {
		auto factory = game->registry<TileEntityFactoryRegistry>().at(json.at("id").get<Identifier>());
		assert(factory);
		auto out = (*factory)();
		out->absorbJSON(game, json);
		return out;
	}

	std::string TileEntity::getSQL() {
		return R"(
			CREATE TABLE IF NOT EXISTS tileEntities (
				globalID INT8 PRIMARY KEY,
				realmID INT,
				row INT8,
				column INT8,
				tileID VARCHAR(255),
				tileEntityID VARCHAR(255),
				encoded MEDIUMBLOB
			);
		)";
	}

	void TileEntity::init(Game &game) {
		assert(!initialized);
		initialized = true;

		auto lock = game.allAgents.uniqueLock();
		assert(!game.allAgents.contains(globalID));
		game.allAgents[globalID] = shared_from_this();
	}

	void TileEntity::tick(const TickArgs &) {
		tryBroadcast();
	}

	void TileEntity::render(SpriteRenderer &sprite_renderer) {
		if (!isVisible())
			return;

		RealmPtr realm = getRealm();
		Tileset &tileset = realm->getTileset();

		if (cachedTile == TileID(-1) || tileLookupFailed) {
			if (tileID.empty()) {
				tileLookupFailed = true;
				cachedTile = 0;
				cachedUpperTile = 0;
			} else {
				tileLookupFailed = false;
				cachedTile = tileset[tileID];
				cachedUpperTile = tileset.getUpper(cachedTile);
				if (cachedUpperTile == 0)
					cachedUpperTile = -1;
			}
		}

		if (cachedTile == 0)
			return;

		GamePtr game = realm->getGame();
		const auto tilesize = tileset.getTileSize();
		const auto texture = tileset.getTexture(*game);
		const auto x = (cachedTile % (texture->width / tilesize)) * tilesize;
		const auto y = (cachedTile / (texture->width / tilesize)) * tilesize;

		sprite_renderer(texture, {
			.x = float(position.column),
			.y = float(position.row),
			.offsetX = x / 2.f,
			.offsetY = y / 2.f,
			.sizeX = float(tilesize),
			.sizeY = float(tilesize),
		});
	}

	void TileEntity::renderUpper(SpriteRenderer &sprite_renderer) {
		if (!isVisible({-1, 0}))
			return;

		RealmPtr realm = getRealm();
		Tileset &tileset = realm->getTileset();

		if (cachedUpperTile == TileID(-1) || cachedUpperTile == 0)
			return;

		GamePtr game = realm->getGame();
		const auto tilesize = tileset.getTileSize();
		const auto texture = tileset.getTexture(*game);
		const auto x = (cachedUpperTile % (texture->width / tilesize)) * tilesize;
		const auto y = (cachedUpperTile / (texture->width / tilesize)) * tilesize;

		sprite_renderer(texture, {
			.x = float(position.column),
			.y = float(position.row - 1),
			.offsetX = x / 2.f,
			.offsetY = y / 2.f,
			.sizeX = float(tilesize),
			.sizeY = float(tilesize),
		});
	}

	void TileEntity::renderLighting(const RendererContext &) {}

	void TileEntity::onOverlap(const EntityPtr &) {}

	void TileEntity::onOverlapEnd(const EntityPtr &) {}

	void TileEntity::onSpawn() {
		GamePtr game = getRealm()->getGame();
		if (game->getSide() == Side::Server)
			game->toServer().tileEntitySpawned(getSelf());
	}

	void TileEntity::onRemove() {
		GamePtr game = getRealm()->getGame();
		if (game->getSide() == Side::Client)
			game->toClient().moduleMessage({}, shared_from_this(), "TileEntityRemoved", Side::Client);
	}

	void TileEntity::setRealm(const RealmPtr &realm) {
		realmID = realm->id;
		weakRealm = realm;
	}

	RealmPtr TileEntity::getRealm() const {
		RealmPtr out = weakRealm.lock();
		if (!out)
			throw std::runtime_error("Couldn't lock tile entity's realm");
		return out;
	}

	void TileEntity::updateNeighbors() const {
		getRealm()->updateNeighbors(position, Layer::Submerged);
		getRealm()->updateNeighbors(position, Layer::Objects);
	}

	bool TileEntity::isVisible() const {
		const Position pos = getPosition();
		RealmPtr realm = getRealm();
		if (getSide() == Side::Client) {
			GamePtr game = realm->getGame();
			ClientGame &client_game = game->toClient();
			return client_game.getWindow()->inBounds(pos) && ChunkRange(client_game.getPlayer()->getChunk()).contains(pos.getChunk());
		}
		return realm->isVisible(pos);
	}

	bool TileEntity::isVisible(const Position &offset) const {
		const Position pos = getPosition() + offset;
		RealmPtr realm = getRealm();
		if (getSide() == Side::Client) {
			GamePtr game = realm->getGame();
			ClientGame &client_game = game->toClient();
			return client_game.getWindow()->inBounds(pos) && ChunkRange(client_game.getPlayer()->getChunk()).contains(pos.getChunk());
		}
		return realm->isVisible(pos);
	}

	Side TileEntity::getSide() const {
		return getRealm()->getGame()->getSide();
	}

	ChunkPosition TileEntity::getChunk() const {
		return getPosition().getChunk();
	}

	std::string TileEntity::getName() const {
		return std::format("Unknown TileEntity ({})", tileEntityID);
	}

	GamePtr TileEntity::getGame() const {
		if (RealmPtr realm = weakRealm.lock())
			return realm->getGame();
		throw std::runtime_error("Couldn't get Game from TileEntity: couldn't lock Realm");
	}

	std::shared_ptr<TileEntity> TileEntity::getSelf() {
		return std::static_pointer_cast<TileEntity>(shared_from_this());
	}

	std::weak_ptr<TileEntity> TileEntity::getWeakSelf() {
		return std::weak_ptr(getSelf());
	}

	void TileEntity::queueDestruction() {
		getRealm()->queueDestruction(getSelf());
	}

	void TileEntity::handleMessage(const std::shared_ptr<Agent> &, const std::string &name, std::any &data) {
		if (name == "GetName") {
			data = Buffer{Side::Client, getName()};
		} else if (name == "GetGID") {
			data = Buffer{Side::Client, getGID()};
		} else if (name == "Encode") {
			Buffer buffer{Side::Client};
			encode(*getGame(), buffer);
			data = std::move(buffer);
		}
	}

	void TileEntity::encode(Game &, Buffer &buffer) {
		buffer << tileEntityID;
		buffer << tileID;
		buffer << position;
		buffer << solid;
		buffer << getUpdateCounter();
		buffer << extraData.dump();
	}

	void TileEntity::decode(Game &, Buffer &buffer) {
		buffer >> tileEntityID;
		buffer >> tileID;
		buffer >> position;
		buffer >> solid;
		setUpdateCounter(buffer.take<UpdateCounter>());
		extraData = nlohmann::json::parse(buffer.take<std::string>());
		cachedTile = -1;
	}

	void TileEntity::sendTo(GenericClient &client, UpdateCounter threshold) {
		assert(getSide() == Side::Server);
		if (threshold == 0 || getUpdateCounter() < threshold) {
			client.send(make<TileEntityPacket>(getSelf()));
			onSend(client.getPlayer());
		}
	}

	void TileEntity::queueBroadcast(bool force) {
		Broadcastable::queueBroadcast(force);

		getGame()->enqueue([weak = getWeakSelf()](const TickArgs &) {
			if (auto tile_entity = weak.lock())
				tile_entity->tryBroadcast();
		});
	}

	void TileEntity::tryBroadcast() {
		if (needsBroadcast.exchange(false))
			broadcast(forceBroadcast.exchange(false));
	}

	void TileEntity::broadcast(bool) {
		assert(getSide() == Side::Server);

		RealmPtr realm = getRealm();
		auto packet = make<TileEntityPacket>(getSelf());

		ChunkRange(getChunk()).iterate([&](ChunkPosition chunk_position) {
			if (auto entities = realm->getEntities(chunk_position)) {
				auto lock = entities->sharedLock();
				for (const WeakEntityPtr &weak_entity: *entities) {
					EntityPtr entity = weak_entity.lock();
					if (entity && entity->isPlayer())
						safeDynamicCast<Player>(entity)->send(packet);
				}
			}
		});
	}

	std::function<void(const TickArgs &)> TileEntity::getTickFunction() {
		return [weak = getWeakSelf()](const TickArgs &args) {
			if (TileEntityPtr tile_entity = weak.lock())
				tile_entity->tick(args);
		};
	}

	Tick TileEntity::enqueueTick(std::chrono::nanoseconds delay) {
		return getGame()->enqueue(getTickFunction(), delay);
	}

	Tick TileEntity::enqueueTick() {
		return getGame()->enqueue(getTickFunction());
	}

	void TileEntity::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
		assert(game->getSide() == Side::Server);
		tileEntityID = json.at("id");
		tileID       = json.at("tileID");
		solid        = json.at("solid");
		if (auto iter = json.find("extra"); iter != json.end())
			extraData = *iter;
		if (auto iter = json.find("position"); iter != json.end())
			position = *iter;
		if (auto iter = json.find("gid"); iter != json.end())
			globalID = *iter;
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

	bool TileEntity::spawnIn(const Place &place) {
		return place.realm->add(getSelf()) != nullptr;
	}

	void to_json(nlohmann::json &json, const TileEntity &tile_entity) {
		tile_entity.toJSON(json);
	}
}
