#include "entity/SquareParticle.h"
#include "graphics/RendererContext.h"
#include "graphics/RenderOptions.h"
#include "graphics/RectangleRenderer.h"
#include "threading/ThreadContext.h"

namespace {
	constexpr double GRAVITY = 32;
}

namespace Game3 {
	SquareParticle::SquareParticle(const Vector3 &initial_velocity, float size, Color color, double depth, double linger_time):
		Entity(ID()), initialVelocity(initial_velocity), size(size), color(color), depth(depth), lingerTime(linger_time) {}

	void SquareParticle::renderShadow(const RendererContext &) {}

	void SquareParticle::render(const RendererContext &renderers) {
		if (!isVisible()) {
			return;
		}

		const auto [row, column] = getPosition();
		const auto [x, y, z] = offset.copyBase();

		renderers.rectangle.drawOnMap({
			.x = double(column) + x + .5,
			.y = double(row) + y - z,
			.sizeX = size,
			.sizeY = size,
			.color = color,
		});
	}

	void SquareParticle::tick(const TickArgs &args) {
		auto offset_lock = offset.uniqueLock();
		auto velocity_lock = velocity.uniqueLock();

		velocity.z -= GRAVITY * args.delta;
		offset.z = std::max(offset.z + args.delta * velocity.z, depth);
		offset.x += args.delta * velocity.x;
		offset.y += args.delta * velocity.y;

		if (offset.z <= depth) {
			velocity = {};
			age += args.delta;
			if (lingerTime <= age) {
				queueDestruction();
				return;
			}
		}

		enqueueTick();
	}

	void SquareParticle::onSpawn() {
		Entity::onSpawn();

		if (initialVelocity) {
			velocity = initialVelocity;
		} else {
			auto lock = velocity.uniqueLock();
			velocity.x = std::uniform_real_distribution(2.,  4.)(threadContext.rng) * (std::uniform_int_distribution(0, 1)(threadContext.rng) == 1? 1 : -1);
			velocity.z = std::uniform_real_distribution(8., 12.)(threadContext.rng);
		}

		if (randomizationOptions) {
			RandomizationOptions &options = *randomizationOptions;
			size = threadContext.random(options.sizeMin, options.sizeMax);
			color = OKHsv{
				threadContext.random(options.hueMin, options.hueMax),
				threadContext.random(options.saturationMin, options.saturationMax),
				threadContext.random(options.valueMin, options.valueMax),
				threadContext.random(options.alphaMin, options.alphaMax),
			}.convert<Color>();
		}
	}

	void SquareParticle::setRandomizationParameters(Buffer buffer) {
		randomizationOptions.emplace(RandomizationOptions{}).decode(buffer);
	}

	int SquareParticle::getZIndex() const {
		return 2;
	}

	void SquareParticle::encode(Buffer &buffer) {
		Entity::encode(buffer);
		buffer << size;
		buffer << color;
		buffer << depth;
		buffer << lingerTime;
	}

	void SquareParticle::decode(Buffer &buffer) {
		Entity::decode(buffer);
		buffer >> size;
		buffer >> color;
		buffer >> depth;
		buffer >> lingerTime;
	}

	void SquareParticle::RandomizationOptions::encode(Buffer &buffer) {
		buffer << sizeMin << sizeMax << hueMin << hueMax << saturationMin << saturationMax << valueMin << valueMax;
	}

	void SquareParticle::RandomizationOptions::decode(Buffer &buffer) {
		buffer >> sizeMin >> sizeMax >> hueMin >> hueMax >> saturationMin >> saturationMax >> valueMin >> valueMax;
	}
}
