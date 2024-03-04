#include "entity/Player.h"
#include "entity/Ship.h"
#include "game/Game.h"
#include "graphics/BatchSpriteRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "realm/Realm.h"
#include "realm/ShipRealm.h"

namespace Game3 {
	namespace {
		constexpr float MOVEMENT_SPEED = 20;
	}

	Ship::Ship():
		Entity(ID()) {}

	void Ship::onSpawn() {
		GamePtr game = getGame();

		if (game->getSide() != Side::Server)
			return;

		internalRealmID = game->newRealmID();
		INFO("Creating ship realm {}", internalRealmID);
		auto ship_realm = Realm::create<ShipRealm>(game, internalRealmID, getGID(), 0);
		game->addRealm(ship_realm);
	}

	void Ship::onDestroy() {
		GamePtr game = getGame();
		if (game->getSide() == Side::Server && internalRealmID != 0)
			game->removeRealm(game->getRealm(internalRealmID));
	}

	void Ship::updateRiderOffset(const EntityPtr &rider) {
		rider->setOffset(getOffset() + Vector3{.5f, .5f, 0.f});
	}

	bool Ship::moveFromRider(const EntityPtr &rider, Direction move_direction, MovementContext context) {
		context.excludePlayer = rider->getGID();
		return move(move_direction, context);
	}

	float Ship::getMovementSpeed() const {
		return MOVEMENT_SPEED;
	}

	bool Ship::canMoveTo(const Place &place) const {
		if (!Entity::canMoveTo(place))
			return false;

		RealmPtr realm = place.realm;
		Position position = getPosition();
		bool out = true;

		iterateTiles([&](const Position &iterated_position) {
			if (!realm->hasFluid(place.position + (iterated_position - position))) {
				out = false;
				return true;
			}

			return false;
		});

		return out;
	}

	bool Ship::onInteractOn(const std::shared_ptr<Player> &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		bool out = false;

		if (modifiers == Modifiers{false, true, true, false}) {
			remove();
			return true;
		}

		const bool player_was_riding = getRider() == player;

		if (player_was_riding) {
			setRider(nullptr);
			out = true;
		}

		if (modifiers.onlyCtrl()) {
			teleportToRealm(player);
			out = true;
		} else if (!player_was_riding) {
			setRider(player);
			out = true;
		}

		return out;
	}

	bool Ship::onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		if (modifiers.onlyCtrl()) {
			teleportToRealm(player);
		} else {
			setRider(player);
		}

		return true;
	}

	void Ship::render(const RendererContext &context) {
		if (!texture || !isVisible())
			return;

		SpriteRenderer &sprite_renderer = context.batchSprite;
		const auto [offset_x, offset_y, offset_z] = offset.copyBase();
		const auto [x_dimension, y_dimension] = getDimensions();
		const auto [row, column] = position.copyBase();

		float texture_x_offset = 16 * (int(direction.load()) - 1);

		const float x = column + offset_x;
		const float y = row    + offset_y - offset_z;

		sprite_renderer(texture, RenderOptions{
			.x = x,
			.y = y,
			.offsetX = texture_x_offset,
			.offsetY = 0.f,
			.sizeX = 16.f * x_dimension,
			.sizeY = 16.f * y_dimension,
		});
	}

	void Ship::encode(Buffer &buffer) {
		Entity::encode(buffer);
		buffer << internalRealmID;
		buffer << realmOrigin;
	}

	void Ship::decode(Buffer &buffer) {
		Entity::decode(buffer);
		buffer >> internalRealmID;
		buffer >> realmOrigin;
	}

	void Ship::teleportToRealm(const std::shared_ptr<Player> &player) {
		if (internalRealmID == 0)
			throw std::runtime_error("Ship is missing internal realm");

		INFO("Internal realm ID: {}", internalRealmID);

		RealmPtr internal_realm = getGame()->tryRealm(internalRealmID);

		if (!internal_realm)
			throw std::runtime_error("Ship is missing internal realm");

		player->teleport(realmOrigin, internal_realm, MovementContext{
			.isTeleport = true,
		});
	}
}
