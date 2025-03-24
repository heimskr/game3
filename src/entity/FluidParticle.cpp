#include "entity/FluidParticle.h"
#include "entity/LivingEntity.h"
#include "game/Game.h"
#include "graphics/CircleRenderer.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/RenderOptions.h"
#include "threading/ThreadContext.h"

namespace Game3 {
	FluidParticle::FluidParticle(FluidID fluidID, const Vector3 &initialVelocity, float size, Color color, double depth, double lingerTime):
		Projectile(ID(), {}, initialVelocity, lingerTime),
		depth(depth),
		size(size),
		color(color),
		fluidID(fluidID) {}

	void FluidParticle::render(const RendererContext &renderers) {
		if (!isVisible()) {
			return;
		}

		const auto [row, column] = getPosition();
		const auto [x, y, z] = offset.copyBase();

		renderers.circle.drawOnMap({
			.x = double(column) + x + .5,
			.y = double(row) + y - z,
			.sizeX = size,
			.sizeY = size,
			.color = color,
		});
	}

	std::shared_ptr<Texture> FluidParticle::getTexture() {
		return {};
	}

	void FluidParticle::onHit(const EntityPtr &target) {
		if (getSide() != Side::Server || hasHit) {
			return;
		}

		if (LivingEntityPtr living = std::dynamic_pointer_cast<LivingEntity>(target)) {
			hasHit = true;

			if (FluidPtr fluid = getGame()->getFluid(fluidID)) {
				fluid->onCollision(living);
			}

			if (living->offset.isGrounded()) { // don't really care about data races here
				applyKnockback(living, 15.0);
			}
		}
	}

	int FluidParticle::getZIndex() const {
		return 2;
	}

	ShadowParams FluidParticle::getShadowParams() const {
		return {
			.baseY = 0.2,
		};
	}

	void FluidParticle::applyKnockback(const EntityPtr &target, float factor) {
		assert(target.get() != this);

		target->velocity.withUnique([this, factor](Vector3 &target_velocity) {
			auto lock = velocity.uniqueLock();
			double magnitude = velocity.magnitude2D();
			target_velocity.x += velocity.x / magnitude * factor;
			target_velocity.y += velocity.y / magnitude * factor;
			velocity.x = 0;
			velocity.y = 0;
			velocity.z = 0;
		});

		target->offset.withUnique([factor](Vector3 &offset) {
			offset.z += factor / 10;
		});

		target->increaseUpdateCounter();
		target->sendToVisible();

		increaseUpdateCounter();
		sendToVisible();
	}

	void FluidParticle::encode(Buffer &buffer) {
		Projectile::encode(buffer);
		buffer << size;
		buffer << color;
		buffer << depth;
		buffer << fluidID;
	}

	void FluidParticle::decode(Buffer &buffer) {
		Projectile::decode(buffer);
		buffer >> size;
		buffer >> color;
		buffer >> depth;
		buffer >> fluidID;
	}
}
