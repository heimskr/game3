#include "entity/TextParticle.h"
#include "graphics/RendererContext.h"
#include "threading/ThreadContext.h"

namespace Game3 {
	TextParticle::TextParticle():
		Entity(ID()) {}

	TextParticle::TextParticle(Glib::ustring text_, Color color_, double linger_time, TextAlign align_):
		Entity(ID()), text(std::move(text_)), color(color_), lingerTime(linger_time), align(align_) {}

	std::shared_ptr<TextParticle> TextParticle::create(const std::shared_ptr<Game> &) {
		return Entity::create<TextParticle>();
	}

	std::shared_ptr<TextParticle> TextParticle::create(const std::shared_ptr<Game> &, Glib::ustring text, Color color, double linger_time, TextAlign align) {
		return Entity::create<TextParticle>(std::move(text), color, linger_time, align);
	}

	void TextParticle::render(const RendererContext &renderers) {
		if (!isVisible() || text.empty())
			return;

		const auto [row, column] = getPosition();
		const auto [x, y, z] = offset.copyBase();

		renderers.text.drawOnMap(text.raw(), {
			.x = double(column) + x + .5,
			.y = double(row) + y - z,
			.color = color,
			.align = align,
		});
	}

	void TextParticle::tick(const TickArgs &args) {
		constexpr static double depth = -1.666;

		auto offset_lock = offset.uniqueLock();

		offset.z = std::max(offset.z + args.delta * velocity.z, depth);

		if (offset.z <= depth) {
			velocity = {};
			age += args.delta;
			if (lingerTime <= age)
				queueDestruction();
			else
				enqueueTick();
			return;
		}

		auto velocity_lock = velocity.uniqueLock();
		velocity.z -= 32 * args.delta;
		offset.x += args.delta * velocity.x;

		enqueueTick();
	}

	void TextParticle::onSpawn() {
		Entity::onSpawn();
		auto lock = velocity.uniqueLock();
		velocity.x = std::uniform_real_distribution(2.,  4.)(threadContext.rng) * (std::uniform_int_distribution(0, 1)(threadContext.rng) == 1? 1 : -1);
		velocity.z = std::uniform_real_distribution(8., 12.)(threadContext.rng);
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
