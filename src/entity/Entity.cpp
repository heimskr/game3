#include "Log.h"
#include "types/Position.h"
#include "graphics/Tileset.h"
#include "data/Identifier.h"
#include "entity/ClientPlayer.h"
#include "entity/Entity.h"
#include "entity/EntityFactory.h"
#include "entity/ServerPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "game/Game.h"
#include "game/ServerGame.h"
#include "game/ServerInventory.h"
#include "graphics/ItemTexture.h"
#include "graphics/RendererContext.h"
#include "graphics/SpriteRenderer.h"
#include "net/Buffer.h"
#include "net/RemoteClient.h"
#include "packet/EntityPacket.h"
#include "packet/EntityRiddenPacket.h"
#include "packet/EntitySetPathPacket.h"
#include "packet/HeldItemSetPacket.h"
#include "realm/Realm.h"
#include "registry/Registries.h"
#include "ui/Window.h"
#include "algorithm/AStar.h"
#include "util/Cast.h"
#include "util/Util.h"

#include <cassert>
#include <iostream>
#include <sstream>

namespace Game3 {
	EntityTexture::EntityTexture(Identifier identifier_, Identifier texture_id, uint8_t variety_):
		NamedRegisterable(std::move(identifier_)),
		textureID(std::move(texture_id)),
		variety(variety_) {}

	EntityPtr Entity::fromJSON(const GamePtr &game, const nlohmann::json &json) {
		auto factory = game->registry<EntityFactoryRegistry>().at(json.at("type").get<EntityType>());
		assert(factory);
		auto out = (*factory)(game, json);
		out->absorbJSON(game, json);
		return out;
	}

	EntityPtr Entity::fromBuffer(const GamePtr &game, Buffer &buffer) {
		const size_t skip = buffer.getSkip();
		EntityType type = buffer.take<EntityType>();
		buffer.setSkip(skip);

		auto factory = game->registry<EntityFactoryRegistry>().at(type);
		assert(factory);
		EntityPtr out = (*factory)(game);
		out->decode(buffer);
		return out;
	}

	std::string Entity::getSQL() {
		return R"(
			CREATE TABLE IF NOT EXISTS entities (
				globalID INT8 PRIMARY KEY,
				realmID INT,
				row INT8,
				column INT8,
				entityType VARCHAR(255),
				direction TINYINT(1),
				encoded MEDIUMBLOB
			);
		)";
	}

	void Entity::destroy() {
		clearQueues();
		onDestroy();
		RealmPtr realm = getRealm();
		EntityPtr shared = getSelf();
		realm->removeSafe(shared);
		GamePtr game = getGame();

		{
			auto &all_agents = game->allAgents;
			auto lock = all_agents.uniqueLock();
			all_agents.erase(globalID);
		}

		realm->eviscerate(shared);

		if (game->getSide() == Side::Server) {
			{
				auto lock = visibleEntities.sharedLock();
				if (!visibleEntities.empty()) {
					for (const auto &weak_visible: visibleEntities)
						if (auto visible = weak_visible.lock())
							visible->removeVisible(shared);
				}
			}

			ServerGame &server_game = game->toServer();
			server_game.getDatabase().deleteEntity(shared);
			server_game.entityDestroyed(*this);
		}
	}

	void Entity::toJSON(nlohmann::json &json) const {
		auto this_lock = sharedLock();
		json["type"]      = type;
		json["position"]  = position;
		json["realmID"]   = realmID;
		json["direction"] = direction;
		json["age"]       = age;
		if (const InventoryPtr inventory = getInventory(0)) {
			// TODO: move JSONification to StorageInventory
			if (getSide() == Side::Client)
				json["inventory"] = static_cast<ClientInventory &>(*inventory);
			else
				json["inventory"] = static_cast<ServerInventory &>(*inventory);
		}

		{
			auto path_lock = path.sharedLock();
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

	void Entity::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
		if (json.is_null())
			return; // Hopefully this is because the Entity is being constructed in EntityPacket::decode.

		auto this_lock = uniqueLock();

		if (auto iter = json.find("type"); iter != json.end())
			type = *iter;
		if (auto iter = json.find("position"); iter != json.end())
			position = iter->get<Position>();
		if (auto iter = json.find("realmID"); iter != json.end())
			realmID = *iter;
		if (auto iter = json.find("direction"); iter != json.end())
			direction = *iter;
		if (auto iter = json.find("inventory"); iter != json.end())
			setInventory(std::make_shared<ServerInventory>(ServerInventory::fromJSON(game, *iter, shared_from_this())), 0);
		if (auto iter = json.find("path"); iter != json.end()) {
			auto path_lock = path.uniqueLock();
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
		if (auto iter = json.find("age"); iter != json.end())
			age = *iter;

		increaseUpdateCounter();
	}

	void Entity::tick(const TickArgs &args) {
		if (!weakRealm.lock()) {
			if (args.game->getSide() == Side::Server)
				teleport(Position{32, 32}, args.game->getRealm(-1));
			tryEnqueueTick();
			return;
		}

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

		const auto delta = args.delta;

		if (!getRidden()) {
			auto offset_lock = offset.uniqueLock();

			auto &x = offset.x;
			auto &y = offset.y;
			auto &z = offset.z;
			const auto speed = getMovementSpeed();

			if (x < 0.)
				x = std::min(x + delta * speed, 0.);
			else if (0. < x)
				x = std::max(x - delta * speed, 0.);

			if (y < 0.)
				y = std::min(y + delta * speed, 0.);
			else if (0. < y)
				y = std::max(y - delta * speed, 0.);

			auto velocity_lock = velocity.uniqueLock();

			bool old_grounded = offset.isGrounded();

			z = std::max(z + delta * velocity.z, 0.);

			if (z > 0) {
				x += delta * velocity.x;
				y += delta * velocity.y;
			} else {
				velocity.x = 0;
				velocity.y = 0;
			}

			if (!old_grounded && offset.isGrounded()) {
				if (TileEntityPtr tile_entity = getRealm()->tileEntityAt(getPosition()))
					tile_entity->onOverlap(getSelf());
			}

			if (z == 0.)
				velocity.z = 0;
			else
				velocity.z -= 32 * delta;

			position.withUnique([&offset = offset](Position &position) {
				using I = Position::IntType;
				position.column += offset.x < 0? -I(-offset.x) : I(offset.x);
				position.row    += offset.y < 0? -I(-offset.y) : I(offset.y);
			});

			double dummy;
			offset.x = std::modf(offset.x, &dummy);
			offset.y = std::modf(offset.y, &dummy);
		}

		// Not all platforms support std::atomic<float>::operator+=.
		age = age + delta;

		if (EntityPtr rider = getRider())
			updateRider(rider);

		tryEnqueueTick();
	}

	void Entity::remove() {
		clearQueues();
		getRealm()->queueDestruction(getSelf());
	}

	void Entity::updateRider(const EntityPtr &rider) {
		updateRiderOffset(rider);
		updateRiderPosition(rider);
	}

	void Entity::updateRiderPosition(const EntityPtr &rider) {
		const Position position = getPosition();
		const RealmPtr realm = getRealm();

		if (rider->getPosition() != position || rider->getRealm() != realm) {
			// const Position difference = position - rider->getPosition();
			rider->teleport(position, realm, MovementContext{
				.clearOffset = false,
				.forcedOffset = getOffset(),
				.suppressPackets = true,
			});
		}
	}

	void Entity::updateRiderOffset(const EntityPtr &rider) {
		rider->setOffset(offset.copyBase());
	}

	void Entity::setRider(const EntityPtr &rider) {
		if (EntityPtr current_rider = getRider())
			current_rider->setRidden(nullptr);

		weakRider = rider;

		if (rider)
			rider->setRidden(getSelf());

		GamePtr game = getGame();

		if (game->getSide() == Side::Server)
			game->toServer().broadcast(EntityRiddenPacket(rider, *this));
	}

	void Entity::setRidden(const EntityPtr &ridden) {
		weakRidden = ridden;
	}

	void Entity::init(const std::shared_ptr<Game> &game) {
		assert(!initialized);
		initialized = true;

		weakGame = game;
		auto shared = shared_from_this();

		{
			auto lock = game->allAgents.uniqueLock();
			if (game->allAgents.contains(globalID)) {
				if (auto locked = game->allAgents.at(globalID).lock()) {
					auto &locked_ref = *locked;
					ERROR("globalID[{}], allAgents<{}>, type[this={}, other={}], this={}, other={}", globalID, game->allAgents.size(), DEMANGLE(*this), DEMANGLE(locked_ref), reinterpret_cast<void *>(this), reinterpret_cast<void *>(locked.get()));
				} else {
					ERROR("globalID[{}], allAgents<{}>, type[this={}, other=expired]", globalID, game->allAgents.size(), DEMANGLE(*this));
				}
				assert(!game->allAgents.contains(globalID));
			}
			game->allAgents[globalID] = shared;
		}

		if (texture == nullptr && game->getSide() == Side::Client) {
			if (customTexture)
				changeTexture(customTexture);
			else
				texture = getTexture();
		}

		InventoryPtr inventory = getInventory(0);

		if (!inventory) {
			setInventory(Inventory::create(getSide(), shared, DEFAULT_INVENTORY_SIZE), 0);
			inventory = getInventory(0);
		} else
			inventory->weakOwner = shared;

		if (game->getSide() == Side::Server) {
			inventory->onSwap = [this](Inventory &here, Slot here_slot, Inventory &there, Slot there_slot) {
				InventoryPtr this_inventory = getInventory(0);
				assert(here == *this_inventory || there == *this_inventory);

				return [this, &here, here_slot, &there, there_slot, this_inventory] {
					for (Held &held: {std::ref(heldLeft), std::ref(heldRight)})
						if (here_slot == held.slot)
							setHeld(here == there? there_slot : -1, held);
				};
			};

			inventory->onMove = [this, weak_inventory = std::weak_ptr(inventory)](Inventory &source, Slot source_slot, Inventory &destination, Slot destination_slot, bool) -> std::function<void()> {
				InventoryPtr inventory = weak_inventory.lock();

				if (!inventory)
					return [] {};

				return [this, &source, source_slot, &destination, destination_slot, inventory] {
					if (source == *inventory && destination == *inventory) {
						for (Held &held: {std::ref(heldLeft), std::ref(heldRight)}) {
							if (source_slot == held.slot) {
								INFO("{}:{}: setHeld(destination_slot[{}], {})", __FILE__, __LINE__, destination_slot, held.isLeft? "left" : "right");
								setHeld(destination_slot, held);
							} else if (destination_slot == held.slot) {
								INFO("{}:{}: setHeld(source_slot[{}], {})", __FILE__, __LINE__, source_slot, held.isLeft? "left" : "right");
								setHeld(source_slot, held);
							}
						}
					} else if (source == *inventory) {
						for (Held &held: {std::ref(heldLeft), std::ref(heldRight)}) {
							if (source_slot == held.slot) {
								INFO("{}:{}: setHeld(-1, {})", __FILE__, __LINE__, held.isLeft? "left" : "right");
								setHeld(-1, held);
							}
						}
					} else {
						assert(destination == *inventory);
						for (Held &held: {std::ref(heldLeft), std::ref(heldRight)}) {
							if (destination_slot == held.slot) {
								INFO("{}:{}: setHeld(-1, {})", __FILE__, __LINE__, held.isLeft? "left" : "right");
								setHeld(-1, held);
							}
						}
					}
				};
			};

			inventory->onRemove = [this](Slot slot) -> std::function<void()> {
				unhold(slot);
				return {};
			};
		}

		movedToNewChunk(std::nullopt);
	}

	void Entity::render(const RendererContext &renderers) {
		if (texture == nullptr || !isVisible())
			return;

		GamePtr game = getGame();
		SpriteRenderer &sprite_renderer = renderers.batchSprite;
		const auto [offset_x, offset_y, offset_z] = offset.copyBase();

		double texture_x_offset = 0.;
		double texture_y_offset = 0.;
		// Animate if the offset is nonzero.
		if (offset_x != 0. || offset_y != 0.) {
			// Choose an animation frame based on the time.
			switch (variety) {
				case 3:
					texture_x_offset = 8. * ((std::chrono::duration_cast<std::chrono::milliseconds>(getTime() - game->startTime).count() / 200) % 4);
					break;
				default:
					texture_x_offset = 8. * ((std::chrono::duration_cast<std::chrono::milliseconds>(getTime() - game->startTime).count() / 100) % 5);
			}
		}

		switch (variety) {
			case 1:
			case 3:
				texture_y_offset = 8. * (int(direction.load()) - 1);
				break;
			case 2:
				texture_y_offset = 16. * (static_cast<int>(remapDirection(direction, 0x1324)) - 1);
				break;
			default:
				break;
		}

		const auto [row, column] = position.copyBase();

		double fluid_offset = 0.;

		if (auto fluid_tile = getRealm()->tileProvider.copyFluidTile({row, column}); fluid_tile && 0 < fluid_tile->level) {
			fluid_offset = std::sin(game->time * 1.5) + .5;
			renderHeight = 10. + fluid_offset;
		} else {
			renderHeight = 16.;
		}


		const double x = column + offset_x;
		const double y = row    + offset_y - offset_z - fluid_offset / 16.;

		RenderOptions main_options{
			.x = x,
			.y = y,
			.offsetX = texture_x_offset,
			.offsetY = texture_y_offset,
			.sizeX = 16.,
			.sizeY = std::min(16., renderHeight + 8. * offset_z),
		};

		if (!heldLeft && !heldRight) {
			sprite_renderer(texture, main_options);
			return;
		}

		auto render_held = [&](const Held &held, double x_o, double y_o, bool flip = false, double degrees = 0.) {
			if (held) {
				sprite_renderer(held.texture, {
					.x = x + x_o,
					.y = y + y_o,
					.offsetX = held.offsetX,
					.offsetY = held.offsetY,
					.sizeX = 16.,
					.sizeY = 16.,
					.scaleX = flip? -.5 : .5,
					.scaleY = .5,
					.angle = degrees,
				});
			}
		};

		constexpr static double rotation = 0.;

		switch (direction) {
			case Direction::Up:
				render_held(heldLeft,  -.1, .4, false, -rotation);
				render_held(heldRight, 1.1, .4, true,   rotation);
				break;
			case Direction::Left:
				render_held(heldRight, 0., .5);
				break;
			case Direction::Right:
				render_held(heldLeft, .5, .5);
				break;
			default:
				break;
		}

		sprite_renderer(texture, main_options);

		switch (direction) {
			case Direction::Down:
				render_held(heldRight, -.1, .5, false, -rotation);
				render_held(heldLeft,  1.1, .5, true,   rotation);
				break;
			case Direction::Left:
				render_held(heldLeft, .5, .5, true);
				break;
			case Direction::Right:
				render_held(heldRight, 1., .5, true);
				break;
			default:
				break;
		}
	}

	void Entity::renderUpper(const RendererContext &) {}

	void Entity::renderLighting(const RendererContext &) {}

	bool Entity::move(Direction move_direction, MovementContext context) {
		if (EntityPtr ridden = getRidden()) {
			EntityPtr self = getSelf();
			if (ridden->moveFromRider(self, move_direction, context)) {
				ridden->updateRider(self);
				return true;
			}

			return false;
		}

		auto self_lock = uniqueLock();

		RealmPtr realm = weakRealm.lock();
		if (!realm) {
			WARN("Can't move entity {}: no realm", getGID());
			return false;
		}

		Position new_position = position;
		double x_offset = 0.;
		double y_offset = 0.;
		bool horizontal = false;
		switch (move_direction) {
			case Direction::Down:
				++new_position.row;
				y_offset = -1.;
				break;
			case Direction::Up:
				--new_position.row;
				y_offset = 1.;
				break;
			case Direction::Left:
				--new_position.column;
				x_offset = 1.;
				horizontal = true;
				break;
			case Direction::Right:
				++new_position.column;
				x_offset = -1.;
				horizontal = true;
				break;
			default:
				throw std::invalid_argument(std::format("Invalid direction: {}", move_direction));
		}

		if (!context.facingDirection)
			context.facingDirection = move_direction;

		if (context.forcedPosition) {
			new_position = *context.forcedPosition;
		} else if ((horizontal && offset.x != 0) || (!horizontal && offset.y != 0)) {
			// WARN("Can't move entity {}: improper offsets [{} : ({}, {})]", globalID, horizontal? "horizontal" : "vertical", offset.x, offset.y);
			return false;
		}

		const bool can_move = canMoveTo({new_position, realm});

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

	bool Entity::moveFromRider(const EntityPtr &, Direction, MovementContext) {
		return false;
	}

	bool Entity::moveFromRider(const EntityPtr &rider, Direction direction) {
		return moveFromRider(rider, direction, MovementContext{});
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

	Entity::Entity(EntityType type):
		type(std::move(type)) {}

	bool Entity::canMoveTo(const Place &place) const {
		RealmPtr realm = place.realm;

		if (!realm)
			return false;

		const Tileset &tileset = realm->getTileset();

		bool out = true;

		iterateTiles([&, position_offset = place.position - getPosition()](const Position &occupied) {
			const Position candidate = occupied + position_offset;

			for (const Layer layer: {Layer::Submerged, Layer::Objects, Layer::Highest}) {
				if (std::optional<TileID> tile = realm->tryTile(layer, candidate); !tile || tileset.isSolid(*tile)) {
					out = false;
					return true;
				}
			}

			if (TileEntityPtr tile_entity = realm->tileEntityAt(candidate)) {
				if (tile_entity->solid) {
					out = false;
					return true;
				}
			}

			return false;
		});

		return out;
	}

	bool Entity::canSpawnAt(const Place &place) const {
		return canMoveTo(place);
	}

	bool Entity::isGrounded() const {
		return getOffset().isGrounded();
	}

	bool Entity::isAffectedByKnockback() const {
		return false;
	}

	void Entity::focus(Window &window, bool is_autofocus) {
		if (EntityPtr ridden = getRidden()) {
			ridden->focus(window, is_autofocus);
			return;
		}

		RealmPtr realm = weakRealm.lock();
		if (!realm)
			return;

		if (!is_autofocus)
			window.scale = 8.;

		constexpr auto map_length = CHUNK_SIZE * REALM_DIAMETER;
		{
			auto lock = offset.sharedLock();
			const auto [row, column] = getPosition();
			window.center.first  = -(column - map_length / 2. + .5) - offset.x;
			window.center.second = -(row    - map_length / 2. + .5) - offset.y;
		}
	}

	bool Entity::teleport(const Position &new_position, MovementContext context) {
		const auto old_chunk_position = position.getChunk();
		const bool in_different_chunk = firstTeleport || old_chunk_position != new_position.getChunk();
		const bool is_server = getSide() == Side::Server;

		if (2 < position.taxiDistance(new_position))
			context.isTeleport = true;

		EntityPtr shared = getSelf();
		RealmPtr realm = getRealm();

		Position old_position = position;
		position = new_position;

		const Vector3 old_offset = getOffset();

		if (context.forcedOffset) {
			offset = *context.forcedOffset;
		} else if (context.clearOffset) {
			if (firstTeleport)
				offset = Vector3{0., 0., 0.};
			else
				offset = Vector3{0., 0., offset.z};
		}

		if (context.facingDirection)
			direction = *context.facingDirection;

		if (is_server)
			increaseUpdateCounter();

		realm->onMoved(shared, old_position, old_offset, new_position, getOffset());

		if (in_different_chunk)
			movedToNewChunk(old_chunk_position);

		{
			auto move_lock = moveQueue.uniqueLock();
			std::erase_if(moveQueue, [shared](const auto &function) {
				return function(shared, false);
			});
		}

		if (is_server && !context.fromPath && !context.suppressPackets) {
			GamePtr game = realm->getGame();
			game->toServer().entityTeleported(*this, context);
		}

		return in_different_chunk;
	}

	void Entity::teleport(const Position &new_position, const std::shared_ptr<Realm> &new_realm, MovementContext context) {
		auto old_realm = weakRealm.lock();
		RealmID limbo_id = inLimboFor.load();

		if ((old_realm != new_realm) || (limbo_id != 0 && limbo_id != new_realm->id)) {
			nextRealm = new_realm->id;
			auto shared = getSelf();

			GamePtr game = getGame();
			if (game->getSide() == Side::Server && old_realm != new_realm && !context.suppressPackets)
				game->toServer().entityChangingRealms(*this, new_realm, new_position);

			if (old_realm && old_realm != new_realm) {
				old_realm->detach(shared);
				old_realm->queueRemoval(shared);
			}

			clearOffset();
			inLimboFor = 0;
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
		return std::format("Entity[type={}, position={}, realm={}, direction={}]", type, position, realmID, direction);
	}

	void Entity::queueForMove(std::function<bool(const EntityPtr &, bool)> function) {
		auto lock = moveQueue.uniqueLock();
		moveQueue.push_back(std::move(function));
	}

	void Entity::queueDestruction() {
		if (!awaitingDestruction.exchange(true))
			getRealm()->queueDestruction(getSelf());
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

		pathfindGoal = goal;
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
			for (const auto &weak_player: visiblePlayers) {
				if (auto player = weak_player.lock()) {
					pathSeers.insert(weak_player);
					player->toServer()->ensureEntity(shared);
					player->send(packet);
				}
			}
		}

		return out == PathResult::Trivial || out == PathResult::Success;
	}

	float Entity::getMovementSpeed() const {
		return isInFluid()? MAX_SPEED * .5f : MAX_SPEED;
	}

	std::shared_ptr<Game> Entity::getGame() const {
		if (auto locked = weakGame.lock())
			return locked;

		GamePtr game = getRealm()->getGame();
		if (!game)
			throw std::runtime_error("Can't lock entity's game");
		weakGame = game;
		return game;
	}

	bool Entity::isVisible() const {
		if (EntityPtr ridden = getRidden(); ridden && ridden->getRideType() == RideType::Hidden)
			return false;

		const auto pos = getPosition();
		auto realm = getRealm();

		GamePtr game = getGame();

		if (game->getSide() == Side::Client) {
			ClientGame &client_game = game->toClient();
			return client_game.getWindow()->inBounds(pos) && ChunkRange(client_game.getPlayer()->getChunk()).contains(pos.getChunk());
		}

		return realm->isVisible(pos);
	}

	bool Entity::isInFluid() const {
		return getRealm()->hasFluid(getPosition());
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

	void Entity::unhold(Slot slot) {
		if (slot < 0)
			return;
		if (heldLeft.slot == slot)
			setHeldLeft(-1);
		if (heldRight.slot == slot)
			setHeldRight(-1);
	}

	Side Entity::getSide() const {
		GamePtr game = getGame();
		return game->getSide();
	}

	void Entity::inventoryUpdated(InventoryID) {
		increaseUpdateCounter();
	}

	ChunkPosition Entity::getChunk() const {
		return getPosition().getChunk();
	}

	bool Entity::canSee(RealmID realm_id, const Position &pos) const {
		const auto realm = getRealm();

		if (realm_id != (nextRealm == 0? realm->id : nextRealm.load()))
			return false;

		const auto this_position = getChunk();
		const auto other_position = pos.getChunk();

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
						players_to_erase.emplace_back(safeDynamicCast<Player>(visible));
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
					for (const WeakEntityPtr &weak_visible: *visible_at_chunk) {
						EntityPtr visible = weak_visible.lock();
						if (!visible || visible.get() == this)
							continue;
						assert(visible->getGID() != getGID());
						visibleEntities.insert(visible);
						if (visible->isPlayer())
							visiblePlayers.emplace(safeDynamicCast<Player>(visible));
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

		if (getSide() != Side::Server)
			return;

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

	size_t Entity::removeVisible(const std::weak_ptr<Entity> &entity) {
		{
			auto lock = visibleEntities.uniqueLock();
			if (visibleEntities.contains(entity)) {
				visibleEntities.erase(entity);
				return 1;
			}
		}

		return 0;
	}

	size_t Entity::removeVisible(const std::weak_ptr<Player> &player) {
		auto players_lock = visiblePlayers.uniqueLock();

		if (auto iter = visiblePlayers.find(player); iter != visiblePlayers.end()) {
			size_t out = 0;
			out += visiblePlayers.erase(iter) != visiblePlayers.end();
			players_lock.unlock();
			{
				auto entities_lock = visibleEntities.uniqueLock();
				out += visibleEntities.erase(player);
			}
			return out;
		}

		return 0;
	}

	void Entity::encode(Buffer &buffer) {
		auto lock = sharedLock();
		buffer << type;
		buffer << globalID;
		buffer << realmID;
		buffer << position;
		buffer << direction.load();
		buffer << getUpdateCounter();
		buffer << offset;
		buffer << velocity;
		buffer << path;
		buffer << money;
		buffer << age;
		// TODO: support multiple entity inventories
		HasInventory::encode(buffer, 0);
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
		buffer >> offset;
		buffer >> velocity;
		buffer >> path;
		buffer >> money;
		buffer >> age;
		// TODO: support multiple entity inventories
		HasInventory::decode(buffer, 0);
		const auto left_slot  = buffer.take<Slot>();
		const auto right_slot = buffer.take<Slot>();

		buffer >> customTexture;
		if (customTexture)
			changeTexture(customTexture);

		setHeldLeft(left_slot);
		setHeldRight(right_slot);
	}

	void Entity::setMoney(MoneyCount new_value) {
		money = new_value;
		if (getSide() == Side::Server) {
			broadcastMoney();
			increaseUpdateCounter();
		}
	}

	void Entity::broadcastMoney() {
		auto lock = visiblePlayers.sharedLock();
		if (visiblePlayers.empty())
			return;

		EntityMoneyChangedPacket packet(*this);
		for (const auto &weak_player: visiblePlayers)
			if (PlayerPtr player = weak_player.lock())
				player->send(packet);
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
		GamePtr game = getGame();
		const bool is_client = game->getSide() == Side::Client;

		if (!is_client) {
			if (RealmPtr realm = weakRealm.lock()) {
				game->toServer().broadcast({position, realm, nullptr}, HeldItemSetPacket(realm->id, getGID(), held.isLeft, new_value, increaseUpdateCounter()));
			}
		}

		if (new_value < 0) {
			held.slot = -1;
			if (is_client)
				held.texture.reset();
			return true;
		}

		const InventoryPtr inventory = getInventory(0);

		if (!inventory->contains(new_value)) {
			WARN("Can't equip slot {}: no item in inventory", new_value);
			held.slot = -1;
			if (is_client)
				held.texture.reset();
			return false;
		}

		held.slot = new_value;

		if (is_client) {
			auto item_texture = game->registry<ItemTextureRegistry>().at((*inventory)[held.slot]->item->identifier);
			held.texture = item_texture->getTexture();
			held.offsetX = item_texture->x / 2.f;
			held.offsetY = item_texture->y / 2.f;
		}

		return true;
	}

	std::shared_ptr<Texture> Entity::getTexture() {
		GamePtr game = getGame();
		auto entity_texture = game->registry<EntityTextureRegistry>().at(type);
		variety = entity_texture->variety;
		return game->registry<TextureRegistry>().at(entity_texture->textureID);
	}

	std::function<void(const TickArgs &)> Entity::getTickFunction() {
		return [weak = getWeakSelf()](const TickArgs &args) {
			if (EntityPtr entity = weak.lock())
				entity->tick(args);
		};
	}

	Tick Entity::enqueueTick(std::chrono::nanoseconds delay) {
		GamePtr game = getGame();
		return tickEnqueued(game->enqueue(getTickFunction(), delay));
	}

	Tick Entity::enqueueTick() {
		GamePtr game = getGame();
		return tickEnqueued(game->enqueue(getTickFunction()));
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

		{
			auto entities_lock = realm->entities.sharedLock();
			auto visible_lock = visibleEntities.uniqueLock();
			visibleEntities.clear();
			for (const EntityPtr &entity: realm->entities)
				if (entity.get() != this && entity->canSee(*this))
					visibleEntities.insert(entity);
		}

		{
			auto players_lock = realm->players.sharedLock();
			auto visible_lock = visiblePlayers.uniqueLock();
			visiblePlayers.clear();
			std::erase_if(realm->players, [this](const std::weak_ptr<Player> &weak_player) {
				if (std::shared_ptr<Player> player = weak_player.lock()) {
					visiblePlayers.emplace(player);
					return false;
				}

				return true;
			});
		}
	}

	void Entity::jump() {
		RealmPtr realm = getRealm();
		GamePtr game = realm->getGame();
		if (game->getSide() != Side::Server || getRidden())
			return;

		{
			auto velocity_lock = velocity.uniqueLock();
			if (velocity.z != 0.)
				return;

			{
				auto offset_lock = offset.sharedLock();
				if (offset.z != 0.)
					return;
			}

			velocity.z = getJumpSpeed();
		}
		increaseUpdateCounter();

		if (TileEntityPtr tile_entity = realm->tileEntityAt(getPosition()))
			tile_entity->onOverlapEnd(getSelf());

		game->toServer().entityTeleported(*this, MovementContext{.excludePlayer = getGID()});
	}

	void Entity::clearOffset() {
		auto lock = offset.uniqueLock();
		offset.x = 0.;
		offset.y = 0.;
		offset.z = 0.;
	}

	EntityPtr Entity::getSelf() {
		return std::static_pointer_cast<Entity>(shared_from_this());
	}

	std::weak_ptr<Entity> Entity::getWeakSelf() {
		return std::weak_ptr(getSelf());
	}

	void Entity::clearQueues() {
		decltype(moveQueue)::Base old_queue;
		{
			auto lock = moveQueue.uniqueLock();
			old_queue = std::move(moveQueue.getBase());
		}

		std::ranges::for_each(old_queue, [shared = getSelf()](const auto &fn) {
			fn(shared, true);
		});
	}

	bool Entity::isInLimbo() const {
		return inLimboFor != 0;
	}

	ItemStackPtr Entity::getHeld(Hand hand) const {
		InventoryPtr inventory = getInventory(0);
		auto lock = inventory->sharedLock();
		switch (hand) {
			case Hand::Left:  return (*inventory)[heldLeft.slot];
			case Hand::Right: return (*inventory)[heldRight.slot];
			default:
				return nullptr;
		}
	}

	Slot Entity::getHeldSlot(Hand hand) const {
		switch (hand) {
			case Hand::Left:  return heldLeft.slot;
			case Hand::Right: return heldRight.slot;
			default:
				return -1;
		}
	}

	Slot Entity::getActiveSlot() const {
		InventoryPtr inventory = getInventory(0);
		auto lock = inventory->sharedLock();
		return inventory->activeSlot;
	}

	bool Entity::isOffsetZero() const {
		constexpr static double EPSILON = 0.001;
		auto lock = offset.sharedLock();
		return offset.x < EPSILON && offset.y < EPSILON && offset.z < EPSILON;
	}

	Vector3 Entity::getOffset() const {
		return offset.copyBase();
	}

	void Entity::setOffset(const Vector3 &new_offset) {
		offset = new_offset;
	}

	void Entity::changeTexture(const Identifier &identifier) {
		GamePtr game = getGame();
		auto entity_texture = game->registry<EntityTextureRegistry>().at(identifier);
		variety = entity_texture->variety;
		texture = game->registry<TextureRegistry>().at(entity_texture->textureID);
		customTexture = identifier;
	}

	void to_json(nlohmann::json &json, const Entity &entity) {
		entity.toJSON(json);
	}
}
