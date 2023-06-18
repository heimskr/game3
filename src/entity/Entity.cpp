#include <cassert>
#include <iostream>
#include <sstream>

#include "Log.h"
#include "Position.h"
#include "Tileset.h"
#include "data/Identifier.h"
#include "entity/Entity.h"
#include "entity/EntityFactory.h"
#include "entity/ServerPlayer.h"
#include "game/ClientGame.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/Buffer.h"
#include "net/RemoteClient.h"
#include "packet/EntityPacket.h"
#include "packet/EntitySetPathPacket.h"
#include "packet/HeldItemSetPacket.h"
#include "realm/Realm.h"
#include "registry/Registries.h"
#include "ui/Canvas.h"
#include "ui/SpriteRenderer.h"
#include "util/AStar.h"
#include "util/Util.h"

namespace Game3 {
	EntityTexture::EntityTexture(Identifier identifier_, Identifier texture_id, uint8_t variety_):
		NamedRegisterable(std::move(identifier_)),
		textureID(std::move(texture_id)),
		variety(variety_) {}

	std::shared_ptr<Entity> Entity::fromJSON(Game &game, const nlohmann::json &json) {
		auto factory = game.registry<EntityFactoryRegistry>().at(json.at("type").get<EntityType>());
		assert(factory);
		auto out = (*factory)(game, json);
		out->absorbJSON(game, json);
		out->init(game);
		return out;
	}

	void Entity::destroy() {
		auto realm = getRealm();
		auto shared = shared_from_this();
		realm->removeSafe(shared);

		{
			auto &all_entities = getGame().allEntities;
			auto lock = all_entities.uniqueLock();
			all_entities.erase(globalID);
		}

		if (getSide() == Side::Server) {
			{
				auto lock = lockVisibleEntitiesShared();
				if (!visibleEntities.empty()) {
					for (const auto &weak_visible: visibleEntities)
						if (auto visible = weak_visible.lock())
							visible->removeVisible(shared);
				}
			}

			getGame().toServer().entityDestroyed(*this);
		}
	}

	void Entity::toJSON(nlohmann::json &json) const {
		json["type"]      = type;
		json["position"]  = position;
		json["realmID"]   = realmID;
		json["direction"] = direction;
		json["health"]    = health;
		if (inventory)
			json["inventory"] = *inventory;
		{
			// I'm sorry. nlohmann forced my hand.
			std::shared_lock lock(const_cast<Entity *>(this)->pathMutex);
			if (!path.empty())
				json["path"] = path;
		}
		if (money != 0)
			json["money"] = money;
		if (0 <= heldLeft.slot)
			json["heldLeft"] = heldLeft.slot;
		if (0 <= heldRight.slot)
			json["heldRight"] = heldRight.slot;
	}

	void Entity::absorbJSON(Game &game, const nlohmann::json &json) {
		if (json.is_null())
			return; // Hopefully this is because the Entity is being constructed in EntityPacket::decode.

		if (auto iter = json.find("type"); iter != json.end())
			type = *iter;
		if (auto iter = json.find("position"); iter != json.end())
			position = iter->get<Position>();
		if (auto iter = json.find("realmID"); iter != json.end())
			realmID = *iter;
		if (auto iter = json.find("direction"); iter != json.end())
			direction = *iter;
		if (auto iter = json.find("health"); iter != json.end())
			health = *iter;
		if (auto iter = json.find("inventory"); iter != json.end())
			inventory = std::make_shared<Inventory>(Inventory::fromJSON(game, *iter, shared_from_this()));
		if (auto iter = json.find("path"); iter != json.end()) {
			std::unique_lock lock(pathMutex);
			path = iter->get<std::list<Direction>>();
		}
		if (auto iter = json.find("money"); iter != json.end())
			money = *iter;
		if (auto iter = json.find("heldLeft"); iter != json.end())
			heldLeft.slot = *iter;
		if (auto iter = json.find("heldRight"); iter != json.end())
			heldRight.slot = *iter;

		increaseUpdateCounter();
	}

	void Entity::tick(Game &, float delta) {
		{
			std::shared_lock shared_lock(pathMutex);
			if (!path.empty() && move(path.front())) {
				// Please no data race kthx.
				shared_lock.unlock();
				std::unique_lock unique_lock(pathMutex);
				if (!path.empty())
					path.pop_front();
			}
		}
		auto &x = offset.x;
		auto &y = offset.y;
		auto &z = offset.z;
		const float speed = getSpeed();

		if (x < 0.f)
			x = std::min(x + delta * speed, 0.f);
		else if (0.f < x)
			x = std::max(x - delta * speed, 0.f);

		if (y < 0.f)
			y = std::min(y + delta * speed, 0.f);
		else if (0.f < y)
			y = std::max(y - delta * speed, 0.f);

		z = std::max(z + delta * zSpeed, 0.f);

		if (z == 0.f) {
			zSpeed = 0.f;
		} else {
			zSpeed -= 32.f * delta;
		}
	}

	void Entity::remove() {
		getRealm()->queueDestruction(shared_from_this());
	}

	void Entity::init(Game &game_) {
		game = &game_;
		auto shared = shared_from_this();

		{
			auto lock = game->allEntities.uniqueLock();
			game->allEntities[globalID] = shared;
		}

		if (texture == nullptr && getSide() == Side::Client)
				texture = getTexture();

		if (!inventory)
			inventory = std::make_shared<Inventory>(shared, DEFAULT_INVENTORY_SIZE);
		else
			inventory->weakOwner = shared;

		movedToNewChunk(std::nullopt);
	}

	void Entity::render(SpriteRenderer &sprite_renderer, TextRenderer &) {
		if (texture == nullptr || !isVisible())
			return;

		float x_offset = 0.f;
		float y_offset = 0.f;
		if (offset.x != 0.f || offset.y != 0.f) {
			switch (variety) {
				case 3:
					x_offset = 8.f * ((std::chrono::duration_cast<std::chrono::milliseconds>(getTime() - getRealm()->getGame().startTime).count() / 200) % 4);
					break;
				default:
					x_offset = 8.f * ((std::chrono::duration_cast<std::chrono::milliseconds>(getTime() - getRealm()->getGame().startTime).count() / 100) % 5);
			}
		}

		switch (variety) {
			case 1:
			case 3:
				y_offset = 8.f * static_cast<int>(direction);
				break;
			case 2:
				y_offset = 16.f * static_cast<int>(remapDirection(direction, 0x0213));
				break;
		}

		if (auto fluid_tile = getRealm()->tileProvider.copyFluidTile(position); fluid_tile && 0 < fluid_tile->level)
			renderHeight = 10.f;
		else
			renderHeight = 16.f;

		const auto x = position.column + offset.x;
		const auto y = position.row    + offset.y - offset.z;
		RenderOptions main_options {
			.x = x,
			.y = y,
			.x_offset = x_offset,
			.y_offset = y_offset,
			.size_x = 16.f,
			.size_y = std::min(16.f, renderHeight + 8.f * offset.z),
		};

		if (!heldLeft && !heldRight) {
			sprite_renderer(*texture, main_options);
			return;
		}

		auto render_held = [&](const Held &held, float x_o, float y_o, bool flip = false, float degrees = 0.f) {
			if (held)
				sprite_renderer(*held.texture, {
					.x = x + x_o,
					.y = y + y_o,
					.x_offset = held.xOffset,
					.y_offset = held.yOffset,
					.size_x = 16.f,
					.size_y = 16.f,
					.scaleX = .5f * (flip? -1 : 1),
					.scaleY = .5f,
					.angle = degrees,
				});
		};

		// constexpr float rotation = 45.f;
		constexpr float rotation = 0.f;

		switch (direction) {
			case Direction::Up:
				render_held(heldLeft,  -.1f, .4f, false, -rotation);
				render_held(heldRight, 1.1f, .4f, true,   rotation);
				break;
			case Direction::Left:
				render_held(heldRight, 0.f, .5f);
				break;
			case Direction::Right:
				render_held(heldLeft, .5f, .5f);
				break;
			default:
				break;
		}

		sprite_renderer(*texture, main_options);

		switch (direction) {
			case Direction::Down:
				render_held(heldRight, -.1f, .5f, false, -rotation);
				render_held(heldLeft,  1.1f, .5f, true,   rotation);
				break;
			case Direction::Left:
				render_held(heldLeft, .5f, .5f, true);
				break;
			case Direction::Right:
				render_held(heldRight, 1.f, .5f, true);
				break;
			default:
				break;
		}
	}

	bool Entity::move(Direction move_direction, std::optional<Direction> new_direction) {
		auto realm = weakRealm.lock();
		if (!realm) {
			if (getSide() == Side::Client) WARN("Can't move entity " << globalID << ": no realm");
			return false;
		}

		Position new_position = position;
		float x_offset = 0.f;
		float y_offset = 0.f;
		bool horizontal = false;
		switch (move_direction) {
			case Direction::Down:
				++new_position.row;
				y_offset = -1.f;
				break;
			case Direction::Up:
				--new_position.row;
				y_offset = 1.f;
				break;
			case Direction::Left:
				--new_position.column;
				x_offset = 1.f;
				horizontal = true;
				break;
			case Direction::Right:
				++new_position.column;
				x_offset = -1.f;
				horizontal = true;
				break;
			default:
				throw std::invalid_argument("Invalid direction: " + std::to_string(int(move_direction)));
		}

		if (!new_direction)
			new_direction = move_direction;

		if ((horizontal && offset.x != 0) || (!horizontal && offset.y != 0)) {
			// WARN("Can't move entity " << globalID << ": improper offsets [" << horizontal << "/" << offset.x << ", " << !horizontal << "/" << offset.y << "]");
			return false;
		}

		const bool can_move = canMoveTo(new_position);
		const bool direction_changed = *new_direction != direction;

		if (can_move || direction_changed) {
			direction = *new_direction;

			if (can_move) {
				if (horizontal)
					offset.x = x_offset;
				else
					offset.y = y_offset;
			}

			bool path_empty = true;
			{
				std::shared_lock lock(pathMutex);
				path_empty = path.empty();
			}
			teleport(can_move? new_position : position, !path_empty, false);

			return true;
		}

		return false;
	}

	std::shared_ptr<Realm> Entity::getRealm() const {
		auto out = weakRealm.lock();
		if (!out)
			throw std::runtime_error("Couldn't lock entity's realm");
		return out;
	}

	Entity & Entity::setRealm(const Game &game, RealmID realm_id) {
		weakRealm = game.realms.at(realm_id);
		realmID = realm_id;
		increaseUpdateCounter();
		return *this;
	}

	Entity & Entity::setRealm(const std::shared_ptr<Realm> realm) {
		weakRealm = realm;
		realmID = realm->id;
		increaseUpdateCounter();
		return *this;
	}

	Entity::Entity(EntityType type_):
		type(type_) {}

	bool Entity::canMoveTo(const Position &new_position) const {
		auto realm = weakRealm.lock();
		if (!realm)
			return false;

		const auto &tileset = realm->getTileset();

		if (auto tile = realm->tryTile(Layer::Terrain, new_position); !tile || !tileset.isWalkable(*tile))
			return false;

		for (const auto layer: {Layer::Submerged, Layer::Objects, Layer::Highest})
			if (auto tile = realm->tryTile(layer, new_position); !tile || tileset.isSolid(*tile))
				return false;

		if (auto tile_entity = realm->tileEntityAt(new_position))
			if (tile_entity->solid)
				return false;

		return true;
	}

	void Entity::focus(Canvas &canvas, bool is_autofocus) {
		auto realm = weakRealm.lock();
		if (!realm)
			return;

		if (!is_autofocus)
			canvas.scale = 4.f;
		else if (++canvas.autofocusCounter < Canvas::AUTOFOCUS_DELAY)
			return;

		canvas.autofocusCounter = 0;
		auto &tileset = realm->getTileset();
		const auto texture = tileset.getTexture(realm->getGame());
		// TODO: fix
		constexpr bool adjust = false; // Render-to-texture silliness
		constexpr auto map_length = CHUNK_SIZE * REALM_DIAMETER;
		canvas.center.x() = -(getColumn() - map_length / 2.f + 0.5f) - offset.x;
		canvas.center.y() = -(getRow()    - map_length / 2.f + 0.5f) - offset.y;
		if (adjust) {
			canvas.center.x() -= canvas.width()  / 32.f / canvas.scale;
			canvas.center.y() += canvas.height() / 32.f / canvas.scale;
		}
	}

	bool Entity::teleport(const Position &new_position, bool from_path, bool clear_offset) {
		const auto old_chunk_position = getChunkPosition(position);
		const bool in_different_chunk = firstTeleport || old_chunk_position != getChunkPosition(new_position);
		const bool is_server = getSide() == Side::Server;

		position = new_position;

		if (clear_offset)
			offset = {0.f, 0.f, offset.z};

		if (is_server)
			increaseUpdateCounter();

		auto shared = shared_from_this();
		getRealm()->onMoved(shared, new_position);

		if (in_different_chunk)
			movedToNewChunk(old_chunk_position);

		for (auto iter = moveQueue.begin(); iter != moveQueue.end();) {
			if ((*iter)(shared))
				moveQueue.erase(iter++);
			else
				++iter;
		}

		if (is_server && !from_path)
			getGame().toServer().entityTeleported(*this);

		return in_different_chunk;
	}

	void Entity::teleport(const Position &new_position, const std::shared_ptr<Realm> &new_realm) {
		bool changing_realms = false;
		auto old_realm = weakRealm.lock();

		if (old_realm != new_realm) {
			changing_realms = true;
			nextRealm = new_realm->id;
			auto shared = shared_from_this();
			if (old_realm) {
				INFO("Moving from " << old_realm->id << " to " << new_realm->id << "; detaching.");
				old_realm->detach(shared);
				old_realm->queueRemoval(shared);
			}

			setRealm(new_realm);

			if (isPlayer() && new_realm->getSide() == Side::Server) {
				auto player = cast<Player>();
				assert(player);
				player->send(RealmNoticePacket(*new_realm));
			}

			new_realm->queueAddition(shared_from_this(), new_position);
		} else {
			teleport(new_position, false);
		}
	}

	Position Entity::nextTo() const {
		switch (direction) {
			case Direction::Up:    return {position.row - 1, position.column};
			case Direction::Down:  return {position.row + 1, position.column};
			case Direction::Left:  return {position.row, position.column - 1};
			case Direction::Right: return {position.row, position.column + 1};
			default: throw std::invalid_argument("Invalid direction: " + std::to_string(int(direction)));
		}
	}

	std::string Entity::debug() const {
		std::stringstream sstream;
		sstream << "Entity[type=" << type << ", position=" << position << ", realm=" << realmID << ", direction=" << direction << ']';
		return sstream.str();
	}

	void Entity::queueForMove(const std::function<bool(const std::shared_ptr<Entity> &)> &function) {
		moveQueue.push_back(function);
	}

	PathResult Entity::pathfind(const Position &start, const Position &goal, std::list<Direction> &out) {
		std::vector<Position> positions;

		if (start == goal)
			return PathResult::Trivial;

		if (!simpleAStar(getRealm(), start, goal, positions))
			return PathResult::Unpathable;

		out.clear();
		pathSeers.clear();

		if (positions.size() < 2)
			return PathResult::Trivial;

		for (auto iter = positions.cbegin() + 1, end = positions.cend(); iter != end; ++iter) {
			const Position &prev = *(iter - 1);
			const Position &next = *iter;
			if (next.row == prev.row + 1)
				out.push_back(Direction::Down);
			else if (next.row == prev.row - 1)
				out.push_back(Direction::Up);
			else if (next.column == prev.column + 1)
				out.push_back(Direction::Right);
			else if (next.column == prev.column - 1)
				out.push_back(Direction::Left);
			else
				throw std::runtime_error("Invalid path offset: " + std::string(next - prev));
		}

		return PathResult::Success;
	}

	bool Entity::pathfind(const Position &goal) {
		PathResult out = PathResult::Invalid;
		{
			std::unique_lock lock(pathMutex);
			out = pathfind(position, goal, path);
		}

		if (out == PathResult::Success && getSide() == Side::Server) {
			increaseUpdateCounter();
			auto shared = shared_from_this();
			const EntitySetPathPacket packet(*this);
			auto lock = lockVisibleEntitiesShared();
			// INFO("visiblePlayers<" << visiblePlayers.size() << ">");
			for (const auto &weak_player: visiblePlayers) {
				if (auto player = weak_player.lock()) {
					pathSeers.insert(weak_player);
					// INFO("Sending EntitySetPath packet");
					player->toServer()->ensureEntity(shared);
					player->send(packet);
				}
			}
		}

		return out == PathResult::Trivial || out == PathResult::Success;
	}

	Game & Entity::getGame() {
		if (game == nullptr)
			game = &getRealm()->getGame();
		return *game;
	}

	Game & Entity::getGame() const {
		if (game != nullptr)
			return *game;

		return getRealm()->getGame();
	}

	bool Entity::isVisible() const {
		const auto pos = getPosition();
		auto realm = getRealm();
		if (getSide() == Side::Client)
			return realm->getGame().toClient().canvas.inBounds(pos) && realm->isVisible(pos);
		return realm->isVisible(pos);
	}

	void Entity::setHeldLeft(Slot new_value) {
		if (0 <= new_value && heldRight.slot == new_value)
			setHeld(-1, heldRight);
		setHeld(new_value, heldLeft);
	}

	void Entity::setHeldRight(Slot new_value) {
		if (0 <= new_value && heldLeft.slot == new_value)
			setHeld(-1, heldLeft);
		setHeld(new_value, heldRight);
	}

	Side Entity::getSide() const {
		return getGame().getSide();
	}

	void Entity::inventoryUpdated() {
		increaseUpdateCounter();
	}

	ChunkPosition Entity::getChunk() const {
		return getChunkPosition(getPosition());
	}

	bool Entity::canSee(RealmID realm_id, const Position &pos) const {
		const auto realm = getRealm();

		if (realm_id != (nextRealm == -1? realm->id : nextRealm))
			return false;

		const auto this_position = getChunk();
		const auto other_position = getChunkPosition(pos);

		if (this_position.x - REALM_DIAMETER / 2 <= other_position.x && other_position.x <= this_position.x + REALM_DIAMETER / 2)
			if (this_position.y - REALM_DIAMETER / 2 <= other_position.y && other_position.y <= this_position.y + REALM_DIAMETER / 2)
				return true;

		return false;
	}

	bool Entity::canSee(const Entity &entity) const {
		return canSee(entity.realmID, entity.getPosition());
	}

	bool Entity::canSee(const TileEntity &tile_entity) const {
		return canSee(tile_entity.realmID, tile_entity.getPosition());
	}

	void Entity::movedToNewChunk(const std::optional<ChunkPosition> &) {
		if (getSide() != Side::Server)
			return;

		auto lock = lockVisibleEntities();
		auto shared = shared_from_this();

		std::vector<std::weak_ptr<Entity>> entities_to_erase;
		entities_to_erase.reserve(visibleEntities.size());

		std::vector<std::weak_ptr<Player>> players_to_erase;
		players_to_erase.reserve(visibleEntities.size() / 20);

		for (const auto &weak_visible: visibleEntities) {
			if (auto visible = weak_visible.lock()) {
				if (!canSee(*visible)) {
					assert(visible.get() != this);
					entities_to_erase.push_back(visible);
					auto other_lock = visible->lockVisibleEntities();
					visible->visibleEntities.erase(shared);
					if (visible->isPlayer())
						players_to_erase.push_back(std::dynamic_pointer_cast<Player>(visible));
				}
			}
		}

		for (const auto &entity: entities_to_erase)
			visibleEntities.erase(entity);

		for (const auto &player: players_to_erase)
			visiblePlayers.erase(player);

		if (auto realm = weakRealm.lock()) {
			const auto this_player = std::dynamic_pointer_cast<Player>(shared);
			// Go through each chunk now visible and update both this entity's visible sets and the visible sets
			// of all the entities in each chunk.
			ChunkRange(getChunk()).iterate([this, realm, shared, this_player](ChunkPosition chunk_position) {
				if (auto visible_at_chunk = realm->getEntities(chunk_position)) {
					auto chunk_lock = visible_at_chunk->sharedLock();
					for (const auto &visible: *visible_at_chunk) {
						if (visible.get() == this)
							continue;
						assert(visible->getGID() != getGID());
						visibleEntities.insert(visible);
						if (visible->isPlayer())
							visiblePlayers.insert(std::dynamic_pointer_cast<Player>(visible));
						auto other_lock = visible->lockVisibleEntities();
						visible->visibleEntities.insert(shared);
						if (this_player)
							visible->visiblePlayers.insert(this_player);
					}
				}
			});
		}

		std::shared_lock path_lock(pathMutex);

		if (!path.empty()) {
			lock.unlock();
			auto shared_lock = lockVisibleEntitiesShared();

			if (!visiblePlayers.empty()) {
				auto shared = shared_from_this();
				const EntitySetPathPacket packet(*this);
				for (const auto &weak_player: visiblePlayers) {
					if (auto player = weak_player.lock(); player && !hasSeenPath(player)) {
						INFO("Late sending EntitySetPathPacket (Entity)");
						player->toServer()->ensureEntity(shared);
						player->send(packet);
						setSeenPath(player);
					}
				}
			}
		}
	}

	bool Entity::hasSeenPath(const PlayerPtr &player) {
		std::shared_lock lock(pathSeersMutex);
		return pathSeers.contains(player);
	}

	void Entity::setSeenPath(const PlayerPtr &player, bool seen) {
		std::unique_lock lock(pathSeersMutex);

		if (seen)
			pathSeers.insert(player);
		else
			pathSeers.erase(player);
	}

	bool Entity::removeVisible(const std::weak_ptr<Entity> &entity) {
		auto lock = lockVisibleEntitiesShared();
		if (visibleEntities.contains(entity)) {
			lock.unlock();
			auto unique_lock = lockVisibleEntities();
			visibleEntities.erase(entity);
			return true;
		}

		return false;
	}

	bool Entity::removeVisible(const std::weak_ptr<Player> &player) {
		auto lock = lockVisibleEntitiesShared();
		if (visiblePlayers.contains(player)) {
			lock.unlock();
			auto unique_lock = lockVisibleEntities();
			visiblePlayers.erase(player);
			visibleEntities.erase(player);
			return true;
		}

		return false;
	}

	void Entity::encode(Buffer &buffer) {
		buffer << type;
		buffer << globalID;
		buffer << realmID;
		buffer << position;
		buffer << direction;
		buffer << getUpdateCounter();
		buffer << offset.x;
		buffer << offset.y;
		buffer << offset.z;
		buffer << zSpeed;
		{
			std::shared_lock lock(pathMutex);
			buffer << path;
		}
		buffer << money;
		buffer << health;
		HasInventory::encode(buffer);
		buffer << heldLeft.slot;
		buffer << heldRight.slot;
	}

	void Entity::decode(Buffer &buffer) {
		buffer >> type;
		setGID(buffer.take<GlobalID>());
		buffer >> realmID;
		buffer >> position;
		buffer >> direction;
		setUpdateCounter(buffer.take<UpdateCounter>());
		buffer >> offset.x;
		buffer >> offset.y;
		buffer >> offset.z;
		buffer >> zSpeed;
		{
			std::unique_lock lock(pathMutex);
			buffer >> path;
		}
		buffer >> money;
		buffer >> health;
		HasInventory::decode(buffer);
		const auto left_slot  = buffer.take<Slot>();
		const auto right_slot = buffer.take<Slot>();
		setHeldLeft(left_slot);
		setHeldRight(right_slot);
	}

	void Entity::sendTo(RemoteClient &client, UpdateCounter threshold) {
		if (threshold == 0 || getUpdateCounter() < threshold) {
			client.send(EntityPacket(shared_from_this()));
			onSend(client.getPlayer());
		}
	}

	void Entity::setHeld(Slot new_value, Held &held) {
		const bool is_client = getSide() == Side::Client;

		if (!is_client)
			getGame().toServer().broadcast({position, getRealm(), nullptr}, HeldItemSetPacket(getRealm()->id, getGID(), held.isLeft, new_value, increaseUpdateCounter()));

		if (new_value < 0) {
			held.slot = -1;
			if (is_client)
				held.texture.reset();
			return;
		}

		if (!inventory->contains(new_value))
			throw std::invalid_argument("Can't equip slot " + std::to_string(new_value) + ": no item in inventory");
		held.slot = new_value;
		auto item_texture = getGame().registry<ItemTextureRegistry>().at((*inventory)[held.slot]->item->identifier);
		held.texture = item_texture->getTexture(getGame());
		held.xOffset = item_texture->x / 2.f;
		held.yOffset = item_texture->y / 2.f;
	}

	std::shared_ptr<Texture> Entity::getTexture() {
		Game &game_ref = getGame();
		auto entity_texture = game_ref.registry<EntityTextureRegistry>().at(type);
		variety = entity_texture->variety;
		return game_ref.registry<TextureRegistry>().at(entity_texture->textureID);
	}

	template <>
	std::vector<Direction> Entity::copyPath<std::vector>() {
		std::vector<Direction> out;
		std::shared_lock lock(pathMutex);
		out.reserve(path.size());
		out = {path.begin(), path.end()};
		return out;
	}

	void Entity::calculateVisibleEntities() {
		if (getSide() != Side::Server)
			return;

		auto realm = weakRealm.lock();
		if (!realm)
			return;

		auto entities_lock = realm->lockEntitiesShared();
		auto visible_lock = lockVisibleEntities();
		visibleEntities.clear();
		visiblePlayers.clear();
		for (const auto &entity: realm->entities) {
			if (entity.get() != this && entity->canSee(*this)) {
				visibleEntities.insert(entity);
				if (entity->isPlayer())
					visiblePlayers.insert(std::dynamic_pointer_cast<Player>(entity));
			}
		}
	}

	void Entity::jump() {
		if (getSide() != Side::Server || zSpeed != 0.f || offset.z != 0.f)
			return;

		zSpeed = 8.f;
		increaseUpdateCounter();
		getGame().toServer().entityTeleported(*this);
	}

	void to_json(nlohmann::json &json, const Entity &entity) {
		entity.toJSON(json);
	}
}
