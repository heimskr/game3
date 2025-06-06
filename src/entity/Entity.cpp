#include "algorithm/AStar.h"
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
#include "graphics/CircleRenderer.h"
#include "graphics/ItemTexture.h"
#include "graphics/RendererContext.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "lib/JSON.h"
#include "net/Buffer.h"
#include "net/GenericClient.h"
#include "packet/EntityPacket.h"
#include "packet/EntityRiddenPacket.h"
#include "packet/EntitySetPathPacket.h"
#include "packet/HeldItemSetPacket.h"
#include "packet/UpdateAgentFieldPacket.h"
#include "realm/Realm.h"
#include "registry/Registries.h"
#include "tile/Tile.h"
#include "types/Position.h"
#include "ui/Window.h"
#include "util/Cast.h"
#include "util/ConstexprHash.h"
#include "util/Log.h"
#include "util/Util.h"

#include <cassert>
#include <iostream>
#include <sstream>

namespace Game3 {
	EntityTexture::EntityTexture(Identifier identifier_, Identifier texture_id, uint8_t variety_):
		NamedRegisterable(std::move(identifier_)),
		textureID(std::move(texture_id)),
		variety(variety_) {}

	EntityPtr Entity::fromJSON(const GamePtr &game, const boost::json::value &json) {
		auto factory = game->registry<EntityFactoryRegistry>().at(boost::json::value_to<EntityType>(json.at("type")));
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
				auto locks = getVisibleEntitiesLocks();
				if (visibleEntities && !visibleEntities->empty()) {
					for (const WeakEntityPtr &weak_visible: *visibleEntities) {
						if (EntityPtr visible = weak_visible.lock()) {
							visible->removeVisible(shared);
						}
					}
				}
			}

			ServerGame &server_game = game->toServer();
			server_game.getDatabase().deleteEntity(shared);
			server_game.entityDestroyed(*this);
		}
	}

	void Entity::toJSON(boost::json::value &json) const {
		auto &object = ensureObject(json);
		auto this_lock = sharedLock();
		object["type"] = boost::json::value_from(type);
		object["position"] = boost::json::value_from(position);
		object["realmID"] = realmID;
		object["direction"] = boost::json::value_from(direction);
		object["age"] = age;

		if (const InventoryPtr inventory = getInventory(0)) {
			// TODO: move JSONification to StorageInventory
			if (getSide() == Side::Client) {
				object["inventory"] = boost::json::value_from(static_cast<ClientInventory &>(*inventory));
			} else {
				object["inventory"] = boost::json::value_from(static_cast<ServerInventory &>(*inventory));
			}
		}

		{
			auto path_lock = path.sharedLock();
			if (!path.empty()) {
				object["path"] = boost::json::value_from(path);
			}
		}

		if (money != 0) {
			object["money"] = money;
		}

		if (0 <= heldLeft.slot) {
			object["heldLeft"] = heldLeft.slot;
		}

		if (0 <= heldRight.slot) {
			object["heldRight"] = heldRight.slot;
		}

		if (customTexture) {
			object["customTexture"] = boost::json::value_from(customTexture);
		}
	}

	void Entity::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		if (json.is_null()) {
			return; // Hopefully this is because the Entity is being constructed in EntityPacket::decode.
		}

		const auto &object = json.as_object();

		auto this_lock = uniqueLock();

		if (auto iter = object.find("type"); iter != object.end()) {
			type = boost::json::value_to<EntityType>(iter->value());
		}

		if (auto iter = object.find("position"); iter != object.end()) {
			position = boost::json::value_to<Position>(iter->value());
		}

		if (auto iter = object.find("realmID"); iter != object.end()) {
			realmID = boost::json::value_to<RealmID>(iter->value());
		}

		if (auto iter = object.find("direction"); iter != object.end()) {
			direction = boost::json::value_to<Direction>(iter->value());
		}

		if (auto iter = object.find("inventory"); iter != object.end()) {
			setInventory(std::make_shared<ServerInventory>(boost::json::value_to<ServerInventory>(iter->value(), std::pair{game, shared_from_this()})), 0);
		}

		if (auto iter = object.find("path"); iter != object.end()) {
			auto path_lock = path.uniqueLock();
			path = boost::json::value_to<std::deque<Direction>>(iter->value());
		}

		if (auto iter = object.find("money"); iter != object.end()) {
			money = boost::json::value_to<MoneyCount>(iter->value());
		}

		if (auto iter = object.find("heldLeft"); iter != object.end()) {
			heldLeft.slot = boost::json::value_to<Slot>(iter->value());
		}

		if (auto iter = object.find("heldRight"); iter != object.end()) {
			heldRight.slot = boost::json::value_to<Slot>(iter->value());
		}

		if (auto iter = object.find("customTexture"); iter != object.end()) {
			customTexture = boost::json::value_to<Identifier>(iter->value());
		}

		if (auto iter = object.find("age"); iter != object.end()) {
			age = getDouble(iter->value());
		}

		increaseUpdateCounter();
	}

	void Entity::tick(const TickArgs &args) {
		if (!weakRealm.lock()) {
			if (args.game->getSide() == Side::Server) {
				teleport(Position{32, 32}, args.game->getRealm(-1));
			}
			tryEnqueueTick();
			return;
		}

		bool path_drained = false;
		{
			auto shared_lock = path.sharedLock();
			if (!path.empty() && move(path.front())) {
				// Please no data race kthx.
				shared_lock.unlock();
				auto unique_lock = path.uniqueLock();
				if (!path.empty()) {
					path.pop_front();
					path_drained = path.empty();
				}
			}
		}

		if (path_drained && args.game->getSide() == Side::Server) {
			args.game->toServer().entityTeleported(*this, MovementContext{.clearOffset = false});
		}

		const auto delta = args.delta;

		applyMotion(delta);

		// Not all platforms support std::atomic<float>::operator+=.
		age = age + delta;

		if (EntityPtr rider = getRider()) {
			updateRider(rider);
		}

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
		if (EntityPtr current_rider = getRider()) {
			current_rider->setRidden(nullptr);
		}

		weakRider = rider;

		if (rider) {
			rider->setRidden(getSelf());
		}

		GamePtr game = getGame();

		if (game->getSide() == Side::Server) {
			game->toServer().broadcast(make<EntityRiddenPacket>(rider, *this));
		}
	}

	void Entity::setRidden(const EntityPtr &ridden) {
		weakRidden = ridden;
	}

	void Entity::init(const std::shared_ptr<Game> &game) {
		assert(!initialized);
		initialized = true;

		weakGame = game;
		AgentPtr shared = shared_from_this();

		{
			auto lock = game->allAgents.uniqueLock();
			if (game->allAgents.contains(globalID)) {
				if (auto locked = game->allAgents.at(globalID).lock()) {
					auto &locked_ref = *locked;
					ERR("globalID[{}], allAgents<{}>, type[this={}, other={}], this={}, other={}", globalID, game->allAgents.size(), DEMANGLE(*this), DEMANGLE(locked_ref), reinterpret_cast<void *>(this), reinterpret_cast<void *>(locked.get()));
				} else {
					ERR("globalID[{}], allAgents<{}>, type[this={}, other=expired]", globalID, game->allAgents.size(), DEMANGLE(*this));
				}
				assert(!game->allAgents.contains(globalID));
			}
			game->allAgents[globalID] = shared;
		}

		if (texture == nullptr && game->getSide() == Side::Client) {
			if (customTexture) {
				changeTexture(customTexture);
			} else {
				texture = getTexture();
			}
		}

		InventoryPtr inventory = getInventory(0);

		if (!inventory) {
			setInventory(Inventory::create(getSide(), shared, DEFAULT_INVENTORY_SIZE), 0);
			inventory = getInventory(0);
		} else {
			inventory->setOwner(shared);
		}

		if (game->getSide() == Side::Server) {
			inventory->onSwap = [this](Inventory &here, Slot here_slot, Inventory &there, Slot there_slot) {
				InventoryPtr this_inventory = getInventory(0);
				assert(here == *this_inventory || there == *this_inventory);

				return [this, &here, here_slot, &there, there_slot, this_inventory] {
					for (Held &held: {std::ref(heldLeft), std::ref(heldRight)}) {
						if (here_slot == held.slot) {
							setHeld(here == there? there_slot : -1, held);
						}
					}
				};
			};

			inventory->onMove = [this, weak_inventory = std::weak_ptr(inventory)](Inventory &source, Slot source_slot, Inventory &destination, Slot destination_slot, bool) -> std::function<void()> {
				InventoryPtr inventory = weak_inventory.lock();

				if (!inventory) {
					return [] {};
				}

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
		if (texture == nullptr || !isVisible()) {
			return;
		}

		GamePtr game = getGame();
		SpriteRenderer &sprite_renderer = renderers.batchSprite;
		const auto [offset_x, offset_y, offset_z] = offset.copyBase();

		double texture_x_offset = 0.;
		double texture_y_offset = 0.;
		// Animate if the offset is nonzero.
		if (offset_x != 0. || offset_y != 0.) {
			// Choose an animation frame based on the time.
			int64_t milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(getTime() - game->startTime).count();
			switch (variety) {
				case 4:
					texture_x_offset = 8. * (1 + ((milliseconds / 200) % 2));
					break;
				case 3:
					texture_x_offset = 8. * ((milliseconds / 200) % 4);
					break;
				case 2:
					texture_x_offset = 8. * (1 + (milliseconds / 200) % 4);
					break;
				default:
					texture_x_offset = 8. * ((milliseconds / 100) % 5);
			}
		}

		switch (variety) {
			case 1:
			case 3:
			case 4:
				texture_y_offset = 8. * (int(direction.load()) - 1);
				break;
			case 2:
				if (Direction secondary = getSecondaryDirection(); secondary == Direction::Invalid) {
					texture_y_offset = 16. * (static_cast<int>(remapDirection(direction, 0x1324)) - 1);
				} else {
					constexpr std::array rows{0, 0, 1, 7, 4, 4, 3, 5, 1, 3, 2, 2, 7, 5, 6, 6};
					const int row = rows.at(((static_cast<int>(secondary) - 1) << 2) | (static_cast<int>(direction.load()) - 1));
					texture_y_offset = 8. * row;
				}
				break;
			default:
				break;
		}

		const auto [row, column] = position.copyBase();

		double fluid_offset = 0.;

		if (std::optional<FluidTile> fluid_tile = getRealm()->tileProvider.copyFluidTile({row, column}); fluid_tile && 0 < fluid_tile->level) {
			fluid_offset = (std::sin(game->time * 1.5) + .5) / 16.;
			renderHeight = 10. + fluid_offset;
		} else {
			renderHeight = 16.;
		}

		const double x = column + offset_x;
		const double y = row    + offset_y - offset_z - fluid_offset;

		const auto [multiplier, composite] = getColors();

		RenderOptions main_options{
			.x = x,
			.y = y,
			.offsetX = texture_x_offset,
			.offsetY = texture_y_offset,
			.sizeX = 16.,
			.sizeY = std::min(16., renderHeight + 8. * offset_z),
			.color = multiplier,
			.composite = composite,
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

	void Entity::renderShadow(const RendererContext &renderers) {
		if (!isVisible() || isInFluid()) {
			return;
		}

		const ShadowParams params = getShadowParams();
		const auto [offset_x, offset_y, offset_z] = offset.copyBase();

		Vector2d shadow_position(getPosition());
		shadow_position.x += offset_x + params.baseX;
		shadow_position.y += offset_y + params.baseY;
		const double shadow_size = params.sizeMinuend - std::clamp(offset_z / params.sizeDivisor, params.sizeClampMin, params.sizeClampMax);

		renderers.circle.drawOnMap({
			.x = shadow_position.x,
			.y = shadow_position.y,
			.sizeX = shadow_size / params.divisorX,
			.sizeY = shadow_size / params.divisorY,
			.color{"#00000044"},
		});
	}

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

		if (!context.facingDirection) {
			context.facingDirection = move_direction;
		}

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
				if (horizontal) {
					offset.x = x_offset;
				} else {
					offset.y = y_offset;
				}
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
		RealmPtr out = weakRealm.lock();
		if (!out) {
			throw std::runtime_error("Couldn't lock entity's realm");
		}
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

		if (!realm) {
			return false;
		}

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

	std::pair<Color, Color> Entity::getColors() const {
		return {Color{"#ffffff"}, Color{"#00000000"}};
	}

	ShadowParams Entity::getShadowParams() const {
		return {};
	}

	bool Entity::visibilityMatters() const {
		return true;
	}

	void Entity::focus(Window &window, bool is_autofocus) {
		if (EntityPtr ridden = getRidden()) {
			ridden->focus(window, is_autofocus);
			return;
		}

		RealmPtr realm = weakRealm.lock();
		if (!realm) {
			return;
		}

		if (!is_autofocus) {
			window.scale = 8.;
		}

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

		if (2 < position.taxiDistance(new_position)) {
			context.isTeleport = true;
		}

		EntityPtr shared = getSelf();
		RealmPtr realm = getRealm();

		const Position old_position = std::exchange(position, new_position);
		const Vector3 old_offset = getOffset();

		if (context.forcedOffset) {
			offset = *context.forcedOffset;
		} else if (context.clearOffset) {
			if (firstTeleport) {
				offset = Vector3{0., 0., 0.};
			} else {
				offset = Vector3{0., 0., offset.z};
			}
		}

		if (context.facingDirection) {
			direction = *context.facingDirection;
		}

		if (is_server) {
			increaseUpdateCounter();
		}

		realm->onMoved(shared, old_position, old_offset, new_position, getOffset());

		if (in_different_chunk) {
			movedToNewChunk(old_chunk_position);
		}

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
		RealmPtr old_realm = weakRealm.lock();
		RealmID limbo_id = inLimboFor.load();

		if ((old_realm != new_realm) || (limbo_id != 0 && limbo_id != new_realm->id)) {
			nextRealm = new_realm->id;
			EntityPtr self = getSelf();

			GamePtr game = getGame();
			if (game->getSide() == Side::Server && old_realm != new_realm && !context.suppressPackets) {
				game->toServer().entityChangingRealms(*this, new_realm, new_position);
			}

			if (old_realm && old_realm != new_realm) {
				old_realm->detach(self);
				old_realm->queueRemoval(self);
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
		if (!awaitingDestruction.exchange(true)) {
			getRealm()->queueDestruction(getSelf());
		}
	}

	PathResult Entity::pathfind(const Position &start, const Position &goal, std::deque<Direction> &out, size_t loop_max) {
		std::vector<Position> positions;

		if (start == goal) {
			return PathResult::Trivial;
		}

		if (!simpleAStar(getRealm(), start, goal, positions, loop_max)) {
			return PathResult::Unpathable;
		}

		out.clear();
		pathSeers.clear();

		if (positions.size() < 2) {
			return PathResult::Trivial;
		}

		for (auto iter = positions.cbegin() + 1, end = positions.cend(); iter != end; ++iter) {
			const Position &prev = *(iter - 1);
			const Position &next = *iter;
			if (next.row == prev.row + 1) {
				out.push_back(Direction::Down);
			} else if (next.row == prev.row - 1) {
				out.push_back(Direction::Up);
			} else if (next.column == prev.column + 1) {
				out.push_back(Direction::Right);
			} else if (next.column == prev.column - 1) {
				out.push_back(Direction::Left);
			} else {
				throw std::runtime_error("Invalid path offset: " + std::string(next - prev));
			}
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
			const auto packet = make<EntitySetPathPacket>(*this);
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
		return speedMultiplier.load() * (isInFluid()? baseSpeed.load() * .5f : baseSpeed.load());
	}

	std::shared_ptr<Game> Entity::getGame() const {
		if (auto locked = weakGame.lock()) {
			return locked;
		}

		GamePtr game = getRealm()->getGame();
		if (!game) {
			throw std::runtime_error("Can't lock entity's game");
		}
		weakGame = game;
		return game;
	}

	bool Entity::isVisible() const {
		if (EntityPtr ridden = getRidden(); ridden && ridden->getRideType() == RideType::Hidden) {
			return false;
		}

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
		if (0 <= new_value && heldRight.slot == new_value && !setHeld(-1, heldRight)) {
			return false;
		}

		return setHeld(new_value, heldLeft);
	}

	bool Entity::setHeldRight(Slot new_value) {
		if (0 <= new_value && heldLeft.slot == new_value && !setHeld(-1, heldLeft)) {
			return false;
		}

		return setHeld(new_value, heldRight);
	}

	void Entity::unhold(Slot slot) {
		if (slot < 0) {
			return;
		}

		if (heldLeft.slot == slot) {
			setHeldLeft(-1);
		}

		if (heldRight.slot == slot) {
			setHeldRight(-1);
		}
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

		if (realm_id != (nextRealm == 0? realm->id : nextRealm.load())) {
			return false;
		}

		const auto this_position = getChunk();
		const auto other_position = pos.getChunk();

		if (this_position.x - REALM_DIAMETER / 2 <= other_position.x && other_position.x <= this_position.x + REALM_DIAMETER / 2) {
			if (this_position.y - REALM_DIAMETER / 2 <= other_position.y && other_position.y <= this_position.y + REALM_DIAMETER / 2) {
				return true;
			}
		}

		return false;
	}

	bool Entity::canSee(const Entity &entity) const {
		return canSee(entity.realmID, entity.getPosition());
	}

	bool Entity::canSee(const TileEntity &tile_entity) const {
		return canSee(tile_entity.realmID, tile_entity.getPosition());
	}

	void Entity::movedToNewChunk(const std::optional<ChunkPosition> &old_chunk_position) {
		EntityPtr shared = getSelf();

		if (RealmPtr realm = weakRealm.lock()) {
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

		if (visibilityMatters()) {
			bool visible_entities_present = false;
			auto outer_lock = visibleEntities.uniqueLock();

			if (visibleEntities) {
				visible_entities_present = true;
				Lockable<WeakSet<Entity>> &visibles = *visibleEntities;
				std::vector<std::weak_ptr<Player>> players_to_erase;
				players_to_erase.reserve(visibles.size() / 20);
				auto inner_lock = visibles.uniqueLock();

				std::vector<std::weak_ptr<Entity>> entities_to_erase;
				entities_to_erase.reserve(visibles.size());

				for (const auto &weak_visible: visibles) {
					if (auto visible = weak_visible.lock()) {
						if (!canSee(*visible)) {
							assert(visible.get() != this);
							entities_to_erase.push_back(visible);

							{
								auto other_outer_lock = visible->visibleEntities.uniqueLock();
								if (visible->visibleEntities) {
									Lockable<WeakSet<Entity>> &other_visibles = *visible->visibleEntities;
									auto other_inner_lock = other_visibles.uniqueLock();
									other_visibles.erase(shared);
								}
							}

							if (visible->isPlayer()) {
								players_to_erase.emplace_back(safeDynamicCast<Player>(visible));
							}
						}
					}
				}

				for (const auto &entity: entities_to_erase) {
					visibles.erase(entity);
				}

				visiblePlayers.withUnique([&](WeakSet<Player> &visible) {
					for (const auto &player: players_to_erase) {
						visible.erase(player);
					}
				});
			}

			if (!visible_entities_present) {
				visiblePlayers.withUnique([&](WeakSet<Player> &visible) {
					std::erase_if(visible, [&](const std::weak_ptr<Player> &weak_player) {
						if (PlayerPtr player = weak_player.lock()) {
							return !canSee(*player);
						}

						return true;
					});
				});
			}

			if (RealmPtr realm = weakRealm.lock()) {
				const auto this_player = std::dynamic_pointer_cast<Player>(shared);
				// Go through each chunk now visible and update both this entity's visible sets and the visible sets
				// of all the entities in each chunk.

				std::unique_lock players_lock = visiblePlayers.uniqueLock();
				std::pair locks = getVisibleEntitiesLocks();

				ChunkRange(getChunk()).iterate([this, realm, shared, this_player](ChunkPosition chunk_position) {
					if (auto visible_at_chunk = realm->getEntities(chunk_position)) {
						auto chunk_lock = visible_at_chunk->sharedLock();
						for (const WeakEntityPtr &weak_visible: *visible_at_chunk) {
							EntityPtr visible = weak_visible.lock();

							if (!visible || visible.get() == this) {
								continue;
							}

							assert(visible->getGID() != getGID());
							if (visibleEntities) {
								visibleEntities->insert(visible);
							}

							if (visible->isPlayer()) {
								visiblePlayers.emplace(safeDynamicCast<Player>(visible));
							}

							if (visible->otherEntityToLock != globalID) {
								otherEntityToLock = visible->globalID;
								{
									auto other_locks = visible->getVisibleEntitiesLocks();
									if (visible->visibleEntities) {
										visible->visibleEntities->insert(shared);
									}
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
		}

		if (getSide() != Side::Server) {
			return;
		}

		auto path_lock = path.sharedLock();

		if (!path.empty()) {
			auto shared_lock = visiblePlayers.sharedLock();

			if (!visiblePlayers.empty()) {
				const auto packet = make<EntitySetPathPacket>(*this);
				for (const auto &weak_player: visiblePlayers) {
					if (PlayerPtr player = weak_player.lock(); player && !hasSeenPath(player)) {
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

		if (seen) {
			pathSeers.insert(player);
		} else {
			pathSeers.erase(player);
		}
	}

	size_t Entity::removeVisible(const std::weak_ptr<Entity> &entity) {
		auto locks = getVisibleEntitiesLocks();
		if (visibleEntities) {
			if (auto iter = visibleEntities->find(entity); iter != visibleEntities->end()) {
				visibleEntities->erase(iter);
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
				auto locks = getVisibleEntitiesLocks();
				if (visibleEntities) {
					out += visibleEntities->erase(player);
				}
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
		buffer << baseSpeed;
		buffer << speedMultiplier;
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
		const auto left_slot = buffer.take<Slot>();
		const auto right_slot = buffer.take<Slot>();

		buffer >> customTexture;
		buffer >> baseSpeed;
		buffer >> speedMultiplier;

		if (customTexture) {
			changeTexture(customTexture);
		}

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
		broadcastPacket(make<EntityMoneyChangedPacket>(*this));
	}

	void Entity::broadcastPacket(const PacketPtr &packet) {
		if (getSide() != Side::Server) {
			return;
		}

		if (isPlayer()) {
			PlayerPtr player = std::dynamic_pointer_cast<Player>(shared_from_this());
			assert(player != nullptr);
			player->send(packet);
		}

		auto lock = visiblePlayers.sharedLock();
		if (visiblePlayers.empty()) {
			return;
		}

		for (const auto &weak_player: visiblePlayers) {
			if (PlayerPtr player = weak_player.lock()) {
				player->send(packet);
			}
		}
	}

	void Entity::sendTo(GenericClient &client, UpdateCounter threshold) {
		if (threshold != 0 && threshold <= getUpdateCounter()) {
			return;
		}

		PlayerPtr player = client.getPlayer();
		if (player == weakExcludedPlayer.lock()) {
			return;
		}

		player->notifyOfRealm(*getRealm());
		client.send(make<EntityPacket>(getSelf()));
		onSend(player);
	}

	void Entity::sendToVisible() {
		PlayerPtr excluded_player = weakExcludedPlayer.lock();
		auto lock = visiblePlayers.sharedLock();
		for (const auto &weak_player: visiblePlayers) {
			if (PlayerPtr player = weak_player.lock()) {
				if (player != excluded_player) {
					sendTo(*player->toServer()->getClient());
				}
			}
		}
	}

	bool Entity::setHeld(Slot new_value, Held &held) {
		GamePtr game = getGame();
		const bool is_client = game->getSide() == Side::Client;

		if (!is_client) {
			if (RealmPtr realm = weakRealm.lock()) {
				game->toServer().broadcast(Place{position, realm, nullptr}, make<HeldItemSetPacket>(realm->id, getGID(), held.isLeft, new_value, increaseUpdateCounter()));
			}
		}

		if (new_value < 0) {
			held.slot = -1;
			if (is_client) {
				held.texture.reset();
			}
			return true;
		}

		const InventoryPtr inventory = getInventory(0);

		if (!inventory->contains(new_value)) {
			WARN("Can't equip slot {}: no item in inventory", new_value);
			held.slot = -1;
			if (is_client) {
				held.texture.reset();
			}
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
			if (EntityPtr entity = weak.lock()) {
				entity->tick(args);
			}
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

	bool Entity::setField(uint32_t field_name, Buffer &field_value, const PlayerPtr &updater) {
		if (getSide() == Side::Server) {
			return Agent::setField(field_name, field_value, updater);
		}

		switch (field_name) {
			AGENT_FIELD(type, true);
			AGENT_FIELD(position, true);
			AGENT_FIELD(realmID, true);
			AGENT_FIELD(direction, true);
			AGENT_FIELD(offset, true);
			AGENT_FIELD(velocity, true);
			AGENT_FIELD(path, true);
			AGENT_FIELD(age, true);
			AGENT_FIELD(baseSpeed, true);
			AGENT_FIELD(speedMultiplier, true);
			default:
				return Agent::setField(field_name, field_value, updater);
		}
	}

#ifndef __MINGW32__
	template <>
	std::vector<Direction> Entity::copyPath<std::vector>() {
		std::vector<Direction> out;
		auto lock = path.sharedLock();
		out.reserve(path.size());
		out = {path.begin(), path.end()};
		return out;
	}
#endif

	void Entity::calculateVisiblePlayers() {
		if (getSide() != Side::Server) {
			return;
		}

		RealmPtr realm = weakRealm.lock();
		if (!realm) {
			return;
		}

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

	Lockable<WeakSet<Entity>> & Entity::getVisibleEntities(std::unique_lock<DefaultMutex> &outer_lock, bool recalculate) {
		if (getSide() != Side::Server) {
			throw std::runtime_error("getVisibleEntities is server-side only");
		}

		RealmPtr realm = weakRealm.lock();
		if (!realm) {
			throw std::runtime_error("Can't find visible entities of realmless entity");
		}

		outer_lock = visibleEntities.uniqueLock();

		if (recalculate) {
			visibleEntities.reset();
		}

		if (visibleEntities) {
			return *visibleEntities;
		}

		Lockable<WeakSet<Entity>> &visible = visibleEntities.emplace();
		auto inner_lock = visible.uniqueLock();
		auto entities_lock = realm->entities.sharedLock();

		for (const EntityPtr &entity: realm->entities) {
			if (entity.get() != this && entity->canSee(*this)) {
				visible.insert(entity);
			}
		}

		return *visibleEntities;
	}

	std::pair<std::unique_lock<DefaultMutex>, std::unique_lock<DefaultMutex>> Entity::getVisibleEntitiesLocks() {
		std::pair<std::unique_lock<DefaultMutex>, std::unique_lock<DefaultMutex>> locks;
		locks.first = visibleEntities.uniqueLock();
		if (visibleEntities) {
			locks.second = visibleEntities->uniqueLock();
		}
		return locks;
	}

	bool Entity::shouldBroadcastDestruction() const {
		return true;
	}

	void Entity::applyMotion(float delta) {
		if (getRidden()) {
			return;
		}

		auto offset_lock = offset.uniqueLock();

		auto &x = offset.x;
		auto &y = offset.y;
		auto &z = offset.z;
		const auto speed = getMovementSpeed();

		if (x < 0.) {
			x = std::min(x + delta * speed, 0.);
		} else if (0. < x) {
			x = std::max(x - delta * speed, 0.);
		}

		if (y < 0.) {
			y = std::min(y + delta * speed, 0.);
		} else if (0. < y) {
			y = std::max(y - delta * speed, 0.);
		}

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
			if (TileEntityPtr tile_entity = getRealm()->tileEntityAt(getPosition())) {
				tile_entity->onOverlap(getSelf());
			}

			GamePtr game = getGame();

			if (game->getSide() == Side::Server) {
				game->toServer().entityTeleported(*this, MovementContext{
					.excludePlayer = isPlayer()? getGID() : -1,
					.clearOffset = false,
				});
			}
		}

		if (z == 0.) {
			velocity.z = 0;
		} else {
			velocity.z -= 32 * delta;
		}

		position.withUnique([&offset = offset](Position &position) {
			using I = Position::IntType;
			position.column += offset.x < 0? -static_cast<I>(-offset.x) : static_cast<I>(offset.x);
			position.row    += offset.y < 0? -static_cast<I>(-offset.y) : static_cast<I>(offset.y);
		});

		double dummy;
		offset.x = std::modf(offset.x, &dummy);
		offset.y = std::modf(offset.y, &dummy);
	}

	Direction Entity::getSecondaryDirection() const {
		return Direction::Invalid;
	}

	void Entity::jump() {
		RealmPtr realm = getRealm();
		GamePtr game = realm->getGame();

		if (game->getSide() != Side::Server || getRidden()) {
			return;
		}

		{
			auto velocity_lock = velocity.uniqueLock();
			if (velocity.z > 0.0) {
				return;
			}

			velocity.z = getJumpSpeed();
		}

		increaseUpdateCounter();

		EntityPtr self = getSelf();

		if (TileEntityPtr tile_entity = realm->tileEntityAt(getPosition())) {
			tile_entity->onOverlapEnd(self);
		}

		Place place = getPlace();

		for (Layer layer: allLayers) {
			if (TilePtr tile = place.getTile(layer)) {
				tile->jumpedFrom(self, place, layer);
			}
		}

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

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const Entity &entity) {
		entity.toJSON(json);
	}
}
