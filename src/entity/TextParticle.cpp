#include "entity/TextParticle.h"
#include "graphics/RendererSet.h"
#include "threading/ThreadContext.h"

namespace Game3 {
	TextParticle::TextParticle():
		Entity(ID()) {}

	TextParticle::TextParticle(Glib::ustring text_, Color color_, float linger_time, TextAlign align_):
		Entity(ID()), text(std::move(text_)), color(color_), lingerTime(linger_time), align(align_) {}

	std::shared_ptr<TextParticle> TextParticle::create(Game &) {
		return Entity::create<TextParticle>();
	}

	std::shared_ptr<TextParticle> TextParticle::create(Game &, Glib::ustring text, Color color, float linger_time, TextAlign align) {
		return Entity::create<TextParticle>(std::move(text), color, linger_time, align);
	}

	void TextParticle::render(const RendererSet &renderers) {
		if (!isVisible() || text.empty())
			return;

		const auto [row, column] = getPosition();
		const auto [x, y, z] = offset.copyBase();

		renderers.text.drawOnMap(text.raw(), {
			.x = float(column) + x + .5f,
			.y = float(row) + y - z,
			.color = color,
			.align = align,
		});
	}

	void TextParticle::tick(Game &, float delta) {
		constexpr static float depth = -1.666f;

		auto offset_lock = offset.uniqueLock();

		offset.z = std::max(offset.z + delta * velocity.z, depth);

		if (offset.z <= depth) {
			velocity = {};
			age += delta;
			if (lingerTime <= age)
				queueDestruction();
			return;
		}


		auto velocity_lock = velocity.uniqueLock();
		velocity.z -= 32 * delta;
		offset.x += delta * velocity.x;
	}

	void TextParticle::onSpawn() {
		Entity::onSpawn();
		auto lock = velocity.uniqueLock();
		velocity.x = std::uniform_real_distribution(2.f,  4.f)(threadContext.rng) * (std::uniform_int_distribution(0, 1)(threadContext.rng) == 1? 1 : -1);
		velocity.z = std::uniform_real_distribution(8.f, 12.f)(threadContext.rng);
	}

	int TextParticle::getZIndex() const {
		return 2;
	}

	void TextParticle::encode(Buffer &buffer) {
		Entity::encode(buffer);
		buffer << text;
		buffer << color;
		buffer << lingerTime;
		buffer << align;
	}

	void TextParticle::decode(Buffer &buffer) {
		Entity::decode(buffer);
		buffer >> text;
		buffer >> color;
		buffer >> lingerTime;
		buffer >> align;
	}
}
