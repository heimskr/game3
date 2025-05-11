#include "entity/ExplosionParticle.h"
#include "game/Game.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "realm/Realm.h"

namespace Game3 {
	namespace {
		constexpr float EXPLOSION_LIFETIME = 0.666;
	}

	ExplosionParticle::ExplosionParticle():
		Entity(ID()) {}

	void ExplosionParticle::tick(const TickArgs &args) {
		age += args.delta;
		if (age >= EXPLOSION_LIFETIME) {
			queueDestruction();
		} else {
			tryEnqueueTick();
		}
	}

	void ExplosionParticle::render(const RendererContext &context) {
		if (!texture || !isVisible()) {
			return;
		}

		SpriteRenderer &sprite_renderer = context.singleSprite;
		const auto [offset_x, offset_y, offset_z] = offset.copyBase();
		const auto [row, column] = position.copyBase();

		int stage = std::min<int>(13, age * 13 / EXPLOSION_LIFETIME);
		float texture_x_offset = 8 * (stage % 4);
		float texture_y_offset = 8 * (stage / 4);

		INFO("({}, {})", texture_x_offset, texture_y_offset);

		const float x = column + offset_x;
		const float y = row    + offset_y - offset_z;

		sprite_renderer(texture, RenderOptions{
			.x = x - texture_x_offset / 16,
			.y = y - texture_y_offset / 16,
			.offsetX = texture_x_offset,
			.offsetY = texture_y_offset,
			.sizeX = 16.f,
			.sizeY = 16.f,
		});
	}

	void ExplosionParticle::encode(Buffer &buffer) {
		Entity::encode(buffer);
		buffer << age;
	}

	void ExplosionParticle::decode(Buffer &buffer) {
		Entity::decode(buffer);
		buffer >> age;
	}
}
