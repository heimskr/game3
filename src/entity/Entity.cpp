#include "Log.h"
#include "Position.h"
#include "Tileset.h"
#include "data/Identifier.h"
#include "entity/Entity.h"
#include "entity/EntityFactory.h"
#include "entity/ServerPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "game/Game.h"
#include "game/ServerGame.h"
#include "game/ServerInventory.h"
#include "graphics/SpriteRenderer.h"
#include "net/Buffer.h"
#include "net/RemoteClient.h"
#include "packet/EntityPacket.h"
#include "packet/EntitySetPathPacket.h"
#include "packet/HeldItemSetPacket.h"
#include "realm/Realm.h"
#include "registry/Registries.h"
#include "ui/Canvas.h"
#include "util/AStar.h"
#include "util/Util.h"

#include <cassert>
#include <iostream>
#include <sstream>

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
		return out;
	}

	void Entity::destroy() {
		clearQueues();
		auto realm = getRealm();
		auto shared = getSelf();
		realm->removeSafe(shared);

		{
			auto &all_agents = getGame().allAgents;
			auto lock = all_agents.uniqueLock();
			all_agents.erase(globalID);
		}

		if (getSide() == Side::Server) {
			{
				auto lock = visibleEntities.sharedLock();
				if (!visibleEntities.empty()) {
					for (const auto &weak_visible: visibleEntities)
						if (auto visible = weak_visible.lock())
							visible->removeVisible(shared);
				}
			}

			ServerGame &game = getGame().toServer();
			game.database.deleteEntity(shared);
			game.entityDestroyed(*this);
		}
	}

	void Entity::toJSON(nlohmann::json &json) const {
		auto lock = sharedLock();
		json["type"]      = type;
		json["position"]  = position;
		json["realmID"]   = realmID;
		json["direction"] = direction;
		json["health"]    = health;
		if (const InventoryPtr inventory = getInventory()) {
			// TODO: move JSONification to StorageInventory
			if (getSide() == Side::Client)
				json["inventory"] = static_cast<ClientInventory &>(*inventory);
			else
				json["inventory"] = static_cast<ServerInventory &>(*inventory);
		}

		{
			auto lock = path.sharedLock();
			if (!path.empty())
				json["path"] = path;
		}

		if (money != 0)
			json["money"] = money;
		if (0 <= heldLeft.slot)
			json["heldLeft"] = heldLeft.slot;
		if (0 <= heldRight.slot)
			json["heldRight"] = heldRight.slot;
		if (customTexture)
			json["customTexture"] = customTexture;
	}

	void Entity::absorbJSON(Game &game, const nlohmann::json &json) {
		if (json.is_null())
			return; // Hopefully this is because the Entity is being constructed in EntityPacket::decode.

		auto lock = uniqueLock();

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
			setInventory(std::make_shared<ServerInventory>(ServerInventory::fromJSON(game, *iter, shared_from_this())));
		if (auto iter = json.find("path"); iter != json.end()) {
			auto lock = path.uniqueLock();
			path = iter->get<std::list<Direction>>();
		}
		if (auto iter = json.find("money"); iter != json.end())
			money = *iter;
		if (auto iter = json.find("heldLeft"); iter != json.end())
			heldLeft.slot = *iter;
		if (auto iter = json.find("heldRight"); iter != json.end())
			heldRight.slot = *iter;
		if (auto iter = json.find("customTexture"); iter != json.end())
			customTexture = *iter;

		increaseUpdateCounter();
	}

	void Entity::tick(Game &, float delta) {
		{
			auto shared_lock = path.sharedLock();
			if (!path.empty() && move(path.front())) {
				// Please no data race kthx.
				shared_lock.unlock();
				auto unique_lock = path.uniqueLock();
				if (!path.empty())
					path.pop_front();
			}
		}

		auto lock = offset.uniqueLock();

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

		if (z == 0.f)
			zSpeed = 0.f;
		else
			// Not all platforms support += and -= for atomic floating point types, unfortunately.
			zSpeed = zSpeed - 32.f * delta;
	}

	void Entity::remove() {
		clearQueues();
		getRealm()->queueDestruction(getSelf());
	}

	void Entity::init(Game &game_) {
		assert(!initialized);
		initialized = true;

		game = &game_;
		auto shared = shared_from_this();

		{
			auto lock = game->allAgents.uniqueLock();
			if (game->allAgents.contains(globalID)) {
				if (auto locked = game->allAgents.at(globalID).lock()) {
					auto &locked_ref = *locked;
					ERROR("globalID[" << globalID << "], allAgents<" << game->allAgents.size() << ">, type[this=" << typeid(*this).name() << ", other=" << typeid(locked_ref).name() << "], this=" << this << ", other=" << locked.get());
				} else {
					ERROR("globalID[" << globalID << "], allAgents<" << game->allAgents.size() << ">, type[this=" << typeid(*this).name() << ", other=expired]");
				}
				assert(!game->allAgents.contains(globalID));
			}
			game->allAgents[globalID] = shared;
		}

		if (texture == nullptr && getSide() == Side::Client) {
			if (customTexture)
				changeTexture(customTexture);
			else
				texture = getTexture();
		}

		InventoryPtr inventory = getInventory();

		if (!inventory) {
			setInventory(Inventory::create(getSide(), shared, DEFAULT_INVENTORY_SIZE));
			inventory = getInventory();
		} else
			inventory->weakOwner = shared;

		if (getSide() == Side::Server) {
			inventory->onSwap = [this](Inventory &here, Slot here_slot, Inventory &there, Slot there_slot) {
				InventoryPtr this_inventory = getInventory();
				assert(here == *this_inventory || there == *this_inventory);

				return [this, &here, here_slot, &there, there_slot, this_inventory] {
					for (Held &held: {std::ref(heldLeft), std::ref(heldRight)})
						if (here_slot == held.slot)
							setHeld(here == there? there_slot : -1, held);
				};
			};

			inventory->onMove = [this](Inventory &source, Slot source_slot, Inventory &destination, Slot destination_slot, bool consumed) {
				return [this, &source, source_slot, &destination, destination_slot, consumed, this_inventory = getInventory()] {
					if (source == *this_inventory && destination == *this_inventory) {
						for (Held &held: {std::ref(heldLeft), std::ref(heldRight)}) {
							if (source_slot == held.slot) {
								INFO(__FILE__ << ':' << __LINE__ << ": setHeld(destination_slot{" << destination_slot << "}, " << (held.isLeft? "left" : "right") << ')');
								setHeld(destination_slot, held);
							} else if (destination_slot == held.slot) {
								INFO(__FILE__ << ':' << __LINE__ << ": setHeld(source_slot{" << destination_slot << "}, " << (held.isLeft? "left" : "right") << ')');
								setHeld(source_slot, held);
							}
						}
					} else if (source == *this_inventory) {
						for (Held &held: {std::ref(heldLeft), std::ref(heldRight)})
							if (source_slot == held.slot) {
								INFO(__FILE__ << ':' << __LINE__ << ": setHeld(-1, " << (held.isLeft? "left" : "right") << ')');
								setHeld(-1, held);
							}
					} else {
						assert(destination == *this_inventory);
						for (Held &held: {std::ref(heldLeft), std::ref(heldRight)})
							if (destination_slot == held.slot) {
								INFO(__FILE__ << ':' << __LINE__ << ": setHeld(-1, " << (held.isLeft? "left" : "right") << ')');
								setHeld(-1, held);
							}
					}
				};
			};
		}

		movedToNewChunk(std::nullopt);
	}

	void Entity::render(SpriteRenderer &sprite_renderer, TextRenderer &) {
		if (texture == nullptr || !isVisible())
			return;

		const auto [offset_x, offset_y, offset_z] = offset.copyBase();

		float texture_x_offset = 0.f;
		float texture_y_offset = 0.f;
		if (offset_x != 0.f || offset_y != 0.f) {
			switch (variety) {
				case 3:
					texture_x_offset = 8.f * ((std::chrono::duration_cast<std::chrono::milliseconds>(getTime() - getRealm()->getGame().startTime).count() / 200) % 4);
					break;
				default:
					texture_x_offset = 8.f * ((std::chrono::duration_cast<std::chrono::milliseconds>(getTime() - getRealm()->getGame().startTime).count() / 100) % 5);
			}
		}

		switch (variety) {
			case 1:
			case 3:
				texture_y_offset = 8.f * (int(direction.load()) - 1);
				break;
			case 2:
				texture_y_offset = 16.f * (static_cast<int>(remapDirection(direction, 0x1324)) - 1);
				break;
		}

		const auto [row, column] = position.copyBase();

		if (auto fluid_tile = getRealm()->tileProvider.copyFluidTile({row, column}); fluid_tile && 0 < fluid_tile->level)
			renderHeight = 10.f;
		else
			renderHeight = 16.f;


		const float x = column + offset_x;
		const float y = row    + offset_y - offset_z;

		RenderOptions main_options{
			.x = x,
			.y = y,
			.xOffset = texture_x_offset,
			.yOffset = texture_y_offset,
			.sizeX = 16.f,
			.sizeY = std::min(16.f, renderHeight + 8.f * offset_z),
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
					.xOffset = held.xOffset,
					.yOffset = held.yOffset,
					.sizeX = 16.f,
					.sizeY = 16.f,
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

	bool Entity::move(Direction move_direction, MovementContext context) {
		auto self_lock = uniqueLock();

		RealmPtr realm = weakRealm.lock();
		if (!realm) {
			WARN("Can't move entity " << getGID() << ": no realm");
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

		if (!context.facingDirection)
			context.facingDirection = move_direction;

		if (context.forcedPosition) {
			new_position = *context.forcedPosition;
		} else if ((horizontal && offset.x != 0) || (!horizontal && offset.y != 0)) {
			// WARN("Can't move entity " << globalID << ": improper offsets [" << (horizontal? "horizontal" : "vertical") << " : (" << offset.x << ", " << offset.y << ")]");
			return false;
		}

		const bool can_move = canMoveTo(new_position);
		const bool direction_changed = *context.facingDirection != direction;

		if (can_move || direction_changed) {
			direction = *context.facingDirection;

			if (context.forcedOffset) {
				offset = *context.forcedOffset;
			} else if (can_move) {
				auto lock = offset.uniqueLock();
				if (horizontal)
					offset.x = x_offset;
				else
					offset.y = y_offset;
			}

			{
				auto lock = path.sharedLock();
				context.fromPath = !path.empty();
			}
			teleport(can_move? new_position : position, context);

			return true;
		}

		return false;
	}

	bool Entity::move(Direction direction) {
		return move(direction, MovementContext{.clearOffset = false});
	}

	std::shared_ptr<Realm> Entity::getRealm() const {
		auto out = weakRealm.lock();
		if (!out)
			throw std::runtime_error("Couldn't lock entity's realm");
		return out;
	}

	Entity & Entity::setRealm(const Game &game, RealmID realm_id) {
		weakRealm = game.getRealm(realm_id);
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
		{
			auto lock = offset.sharedLock();
			const auto [row, column] = getPosition();
			canvas.center.x() = -(column - map_length / 2.f + 0.5f) - offset.x;
			canvas.center.y() = -(row    - map_length / 2.f + 0.5f) - offset.y;
		}
		if (adjust) {
			canvas.center.x() -= canvas.width()  / 32.f / canvas.scale;
			canvas.center.y() += canvas.height() / 32.f / canvas.scale;
		}
	}

	bool Entity::teleport(const Position &new_position, MovementContext context) {
		const auto old_chunk_position = getChunkPosition(position);
		const bool in_different_chunk = firstTeleport || old_chunk_position != getChunkPosition(new_position);
		const bool is_server = getSide() == Side::Server;

		if (2 < position.taxiDistance(new_position))
			context.isTeleport = true;

		position = new_position;

		if (firstTeleport)
			offset = Offset{0.f, 0.f, 0.f};
		else if (context.clearOffset)
			offset = Offset{0.f, 0.f, offset.z};

		if (is_server)
			increaseUpdateCounter();

		auto shared = getSelf();
		getRealm()->onMoved(shared, new_position);

		if (in_different_chunk)
			movedToNewChunk(old_chunk_position);

		{
			auto move_lock = moveQueue.uniqueLock();
			std::erase_if(moveQueue, [shared](const auto &function) {
				return function(shared);
			});
		}

		if (is_server && !context.fromPath)
			getGame().toServer().entityTeleported(*this, context);

		return in_different_chunk;
	}

	void Entity::teleport(const Position &new_position, const std::shared_ptr<Realm> &new_realm, MovementContext context) {
		auto old_realm = weakRealm.lock();
		RealmID limbo_id = inLimboFor.load();

		if ((old_realm != new_realm) || (limbo_id != RealmID(-1) && limbo_id != new_realm->id)) {
			nextRealm = new_realm->id;
			auto shared = getSelf();

			if (getSide() == Side::Server && old_realm != new_realm)
				getGame().toServer().entityChangingRealms(*this, new_realm, new_position);

			if (old_realm && old_realm != new_realm) {
				old_realm->detach(shared);
				old_realm->queueRemoval(shared);
			}

			clearOffset();
			inLimboFor = -1;
			new_realm->queueAddition(getSelf(), new_position);
		} else {
			teleport(new_position, context);
		}
	}

	Position Entity::nextTo() const {
		switch (direction) {
			case Direction::Up:    return {position.row - 1, position.column};
			case Direction::Down:  return {position.row + 1, position.column};
			case Direction::Left:  return {position.row, position.column - 1};
			case Direction::Right: return {position.row, position.column + 1};
			default:
				throw std::invalid_argument("Invalid direction: " + std::to_string(int(direction.load())));
		}
	}

	std::string Entity::debug() const {
		std::stringstream sstream;
		sstream << "Entity[type=" << type << ", position=" << position << ", realm=" << realmID << ", direction=" << direction << ']';
		return sstream.str();
	}

	void Entity::queueForMove(const std::function<bool(const std::shared_ptr<Entity> &)> &function) {
		auto lock = moveQueue.uniqueLock();
		moveQueue.push_back(function);
	}

	PathResult Entity::pathfind(const Position &start, const Position &goal, std::list<Direction> &out, size_t loop_max) {
		std::vector<Position> positions;

		if (start == goal)
			return PathResult::Trivial;

		if (!simpleAStar(getRealm(), start, goal, positions, loop_max))
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

	bool Entity::pathfind(const Position &goal, size_t loop_max) {
		PathResult out = PathResult::Invalid;
		{
			auto lock = path.uniqueLock();
			out = pathfind(position, goal, path, loop_max);
		}

		if (out == PathResult::Success && getSide() == Side::Server) {
			increaseUpdateCounter();
			auto shared = getSelf();
			const EntitySetPathPacket packet(*this);
			auto lock = visiblePlayers.sharedLock();
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

	bool Entity::setHeldLeft(Slot new_value) {
		if (0 <= new_value && heldRight.slot == new_value && !setHeld(-1, heldRight))
			return false;
		return setHeld(new_value, heldLeft);
	}

	bool Entity::setHeldRight(Slot new_value) {
		if (0 <= new_value && heldLeft.slot == new_value && !setHeld(-1, heldLeft))
			return false;
		return setHeld(new_value, heldRight);
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

		if (realm_id != (nextRealm == -1? realm->id : nextRealm.load()))
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

	void Entity::movedToNewChunk(const std::optional<ChunkPosition> &old_chunk_position) {
		if (getSide() != Side::Server)
			return;

		auto shared = getSelf();

		if (auto realm = weakRealm.lock()) {
			if (old_chunk_position) {
				realm->queue([realm, shared, chunk_position = *old_chunk_position] {
					realm->detach(shared, chunk_position);
					realm->attach(shared);
				});
			} else {
				realm->queue([realm, shared] {
					realm->attach(shared);
				});
			}
		}

		auto entities_lock = visibleEntities.uniqueLock();

		std::vector<std::weak_ptr<Entity>> entities_to_erase;
		entities_to_erase.reserve(visibleEntities.size());

		std::vector<std::weak_ptr<Player>> players_to_erase;
		players_to_erase.reserve(visibleEntities.size() / 20);

		for (const auto &weak_visible: visibleEntities) {
			if (auto visible = weak_visible.lock()) {
				if (!canSee(*visible)) {
					assert(visible.get() != this);
					entities_to_erase.push_back(visible);

					{
						auto other_lock = visible->visibleEntities.uniqueLock();
						visible->visibleEntities.erase(shared);
					}

					if (visible->isPlayer())
						players_to_erase.push_back(std::dynamic_pointer_cast<Player>(visible));
				}
			}
		}

		for (const auto &entity: entities_to_erase)
			visibleEntities.erase(entity);

		{
			auto players_lock = visiblePlayers.uniqueLock();

			for (const auto &player: players_to_erase)
				visiblePlayers.erase(player);
		}

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
						if (visible->otherEntityToLock != globalID) {
							otherEntityToLock = visible->globalID;
							{
								auto other_lock = visible->visibleEntities.uniqueLock();
								visible->visibleEntities.insert(shared);
							}
							if (this_player) {
								auto other_lock = visible->visiblePlayers.uniqueLock();
								visible->visiblePlayers.insert(this_player);
							}
							otherEntityToLock = -1;
						} else {
							// The other entity is already handling this.
						}
					}
				}
			});
		}

		auto path_lock = path.sharedLock();

		if (!path.empty()) {
			entities_lock.unlock();
			auto shared_lock = visiblePlayers.sharedLock();

			if (!visiblePlayers.empty()) {
				auto shared = getSelf();
				const EntitySetPathPacket packet(*this);
				for (const auto &weak_player: visiblePlayers) {
					if (auto player = weak_player.lock(); player && !hasSeenPath(player)) {
						// INFO("Late sending EntitySetPathPacket (Entity)");
						player->toServer()->ensureEntity(shared);
						player->send(packet);
						setSeenPath(player);
					}
				}
			}
		}
	}

	bool Entity::hasSeenPath(const PlayerPtr &player) {
		auto lock = pathSeers.sharedLock();
		return pathSeers.contains(player);
	}

	void Entity::setSeenPath(const PlayerPtr &player, bool seen) {
		auto lock = pathSeers.uniqueLock();

		if (seen)
			pathSeers.insert(player);
		else
			pathSeers.erase(player);
	}

	bool Entity::removeVisible(const std::weak_ptr<Entity> &entity) {
		auto shared_lock = visibleEntities.uniqueLock();
		if (visibleEntities.contains(entity)) {
			shared_lock.unlock();
			auto lock = visibleEntities.uniqueLock();
			visibleEntities.erase(entity);
			return true;
		}

		return false;
	}

	bool Entity::removeVisible(const std::weak_ptr<Player> &player) {
		auto shared_lock = visiblePlayers.sharedLock();

		if (visiblePlayers.contains(player)) {
			shared_lock.unlock();
			{
				auto lock = visiblePlayers.uniqueLock();
				visiblePlayers.erase(player);
			}
			{
				auto lock = visibleEntities.uniqueLock();
				visibleEntities.erase(player);
			}
			return true;
		}

		return false;
	}

	void Entity::encode(Buffer &buffer) {
		auto lock = sharedLock();
		buffer << type;
		buffer << globalID;
		buffer << realmID;
		buffer << position;
		buffer << direction.load();
		buffer << getUpdateCounter();
		buffer << offset.x;
		buffer << offset.y;
		buffer << offset.z;
		buffer << zSpeed;
		buffer << path;
		buffer << money;
		buffer << health;
		HasInventory::encode(buffer);
		buffer << heldLeft.slot;
		buffer << heldRight.slot;
		buffer << customTexture;
	}

	void Entity::decode(Buffer &buffer) {
		auto lock = uniqueLock();
		buffer >> type;
		setGID(buffer.take<GlobalID>());
		buffer >> realmID;
		buffer >> position;
		buffer >> direction;
		setUpdateCounter(buffer.take<UpdateCounter>());
		{
			auto offset_lock = offset.uniqueLock();
			buffer >> offset.x;
			buffer >> offset.y;
			buffer >> offset.z;
		}
		buffer >> zSpeed;
		buffer >> path;
		buffer >> money;
		buffer >> health;
		HasInventory::decode(buffer);
		const auto left_slot  = buffer.take<Slot>();
		const auto right_slot = buffer.take<Slot>();

		buffer >> customTexture;
		if (customTexture)
			changeTexture(customTexture);

		setHeldLeft(left_slot);
		setHeldRight(right_slot);
	}

	void Entity::sendTo(RemoteClient &client, UpdateCounter threshold) {
		if (threshold == 0 || getUpdateCounter() < threshold) {
			RealmPtr realm = getRealm();
			client.getPlayer()->notifyOfRealm(*realm);
			client.send(EntityPacket(getSelf()));
			onSend(client.getPlayer());
		}
	}

	void Entity::sendToVisible() {
		auto lock = visiblePlayers.sharedLock();
		for (const auto &weak_player: visiblePlayers)
			if (PlayerPtr player = weak_player.lock())
				sendTo(*player->toServer()->getClient());
	}

	bool Entity::setHeld(Slot new_value, Held &held) {
		const bool is_client = getSide() == Side::Client;

		if (!is_client)
			getGame().toServer().broadcast({position, getRealm(), nullptr}, HeldItemSetPacket(getRealm()->id, getGID(), held.isLeft, new_value, increaseUpdateCounter()));

		if (new_value < 0) {
			held.slot = -1;
			if (is_client)
				held.texture.reset();
			return true;
		}

		const InventoryPtr inventory = getInventory();

		if (!inventory->contains(new_value)) {
			WARN("Can't equip slot " << new_value << ": no item in inventory");
			held.slot = -1;
			if (is_client)
				held.texture.reset();
			return false;
		}

		held.slot = new_value;
		auto item_texture = getGame().registry<ItemTextureRegistry>().at((*inventory)[held.slot]->item->identifier);
		held.texture = item_texture->getTexture(getGame());
		held.xOffset = item_texture->x / 2.f;
		held.yOffset = item_texture->y / 2.f;
		return true;
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
		auto lock = path.sharedLock();
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

		auto entities_lock = realm->entities.sharedLock();
		auto visible_lock = visibleEntities.uniqueLock();
		auto players_lock = visiblePlayers.uniqueLock();
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

		zSpeed = getJumpSpeed();
		increaseUpdateCounter();
		getGame().toServer().entityTeleported(*this, MovementContext{.excludePlayerSelf = true});
	}

	void Entity::clearOffset() {
		auto lock = offset.uniqueLock();
		offset.x = 0.f;
		offset.y = 0.f;
		offset.z = 0.f;
	}

	std::shared_ptr<Entity> Entity::getSelf() {
		return std::static_pointer_cast<Entity>(shared_from_this());
	}

	void Entity::clearQueues() {
		auto lock = moveQueue.uniqueLock();
		moveQueue.clear();
	}

	bool Entity::isInLimbo() const {
		return inLimboFor != RealmID(-1);
	}

	void Entity::changeTexture(const Identifier &identifier) {
		Game &game_ref = getGame();
		auto entity_texture = game_ref.registry<EntityTextureRegistry>().at(identifier);
		variety = entity_texture->variety;
		texture = game_ref.registry<TextureRegistry>().at(entity_texture->textureID);
		customTexture = identifier;
	}

	void to_json(nlohmann::json &json, const Entity &entity) {
		entity.toJSON(json);
	}
}
