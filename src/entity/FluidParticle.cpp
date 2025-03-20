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
				applyKnockback(living, 0.1);
			}
		}
	}

	int FluidParticle::getZIndex() const {
		return 2;
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
