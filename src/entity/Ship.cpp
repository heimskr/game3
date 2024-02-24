#include "entity/Player.h"
#include "entity/Ship.h"
#include "graphics/BatchSpriteRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"

namespace Game3 {
	Ship::Ship():
		Entity(ID()) {}

	void Ship::updateRiderOffset(const EntityPtr &rider) {
		rider->setOffset(getOffset() + Vector3{.5, .5, .0});
	}

	bool Ship::moveFromRider(const EntityPtr &rider, Direction move_direction, MovementContext context) {
		context.excludePlayer = rider->getGID();
		return move(move_direction, context);
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

		double texture_x_offset = 16 * (int(direction.load()) - 1);

		const double x = column + offset_x;
		const double y = row    + offset_y - offset_z;

		sprite_renderer(texture, RenderOptions{
			.x = x,
			.y = y,
			.offsetX = texture_x_offset,
			.offsetY = 0.,
			.sizeX = 16. * x_dimension,
			.sizeY = 16. * y_dimension,
		});
	}

	void Ship::encode(Buffer &buffer) {
		Entity::encode(buffer);
	}

	void Ship::decode(Buffer &buffer) {
		Entity::decode(buffer);
	}
}
