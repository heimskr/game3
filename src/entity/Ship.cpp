#include "entity/Player.h"
#include "entity/Ship.h"
#include "graphics/BatchSpriteRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "realm/Realm.h"

namespace Game3 {
	namespace {
		constexpr float MOVEMENT_SPEED = 20;
	}

	Ship::Ship():
		Entity(ID()) {}

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

	bool Ship::canMoveTo(const Position &new_position) const {
		if (!Entity::canMoveTo(new_position))
			return false;

		RealmPtr realm = getRealm();
		Position position = getPosition();
		bool out = true;

		iterateTiles([&](const Position &iterated_position) {
			if (!realm->hasFluid(new_position + (iterated_position - position))) {
				out = false;
				return true;
			}

			return false;
		});

		return out;
	}

	bool Ship::onInteractOn(const std::shared_ptr<Player> &player, Modifiers, ItemStack *, Hand) {
		if (getRider() == player) {
			setRider(nullptr);
			return true;
		}

		return false;
	}

	bool Ship::onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers, ItemStack *, Hand) {
		setRider(player);
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
	}

	void Ship::decode(Buffer &buffer) {
		Entity::decode(buffer);
	}
}
