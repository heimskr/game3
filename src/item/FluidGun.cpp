#include "Log.h"
#include "entity/Player.h"
#include "entity/SquareParticle.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "graphics/Tileset.h"
#include "item/FluidGun.h"
#include "packet/ClickPacket.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "types/Position.h"

namespace Game3 {
	constexpr static double velocityBase = 2;
	constexpr static double velocityVariance = 0.8;
	constexpr static double jitterScale = 0.2;
	constexpr static double sizeBase = 0.333;
	constexpr static double sizeVariance = 0.8;

	static auto makeParticle(const GamePtr &game, const FluidPtr &fluid, const Place &place, std::pair<float, float> offsets) {
		auto [x_offset, y_offset] = offsets;
		PlayerPtr player = place.player;
		const Vector3 player_offset = player->getOffset();
		const Position relative = place.position - player->getPosition();
		const auto x_jitter = threadContext.random(-jitterScale, jitterScale);
		const auto y_jitter = threadContext.random(-jitterScale, jitterScale);
		const Vector3 jitter(x_jitter, y_jitter, 0);
		Vector3 velocity(relative.column + (0.5 - x_offset), relative.row + (0.5 - y_offset), 16.0);
		velocity -= player_offset * Vector3(1, 1, 0) - jitter;
		const double velocity_scale = threadContext.random(velocityBase * velocityVariance, velocityBase / velocityVariance);
		velocity.x *= velocity_scale;
		velocity.y *= velocity_scale;
		velocity.z /= velocity_scale;
		const double size = threadContext.random(sizeBase * sizeVariance, sizeBase / sizeVariance);
		auto entity = SquareParticle::create(game, velocity, size, fluid->color, 0, 2);
		entity->spawning = true;
		entity->setRealm(place.realm);
		entity->offset = player_offset - jitter;
		return entity;
	}

	bool FluidGun::use(Slot, const ItemStackPtr &stack, const Place &place, Modifiers, std::pair<float, float> offsets) {
		assert(stack != nullptr);
		GamePtr game = place.player->getGame();
		assert(game->getSide() == Side::Server);

		auto fluid = game->getFluid("base:fluid/lava");
		auto entity = makeParticle(game, fluid, place, offsets);
		entity->excludedPlayer = place.player;
		place.realm->queueEntityInit(std::move(entity), place.player->getPosition());
		return true;
	}

	bool FluidGun::fire(Slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets) {
		assert(stack != nullptr);
		GamePtr game = place.player->getGame();
		assert(game->getSide() == Side::Client);

		auto fluid = game->getFluid("base:fluid/lava");
		auto entity = makeParticle(game, fluid, place, offsets);
		place.realm->queueEntityInit(std::move(entity), place.player->getPosition());
		game->toClient().send(make<ClickPacket>(place.position, offsets.first, offsets.second, modifiers));

		{
			static std::chrono::system_clock::time_point last_play{};
			auto now = std::chrono::system_clock::now();
			auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_play).count();
			if (diff > 80) {
				last_play = now;
				constexpr static float variance = .8;
				place.realm->playSound(place.position, "base:sound/hit", std::uniform_real_distribution(variance, 1.f / variance)(threadContext.rng));
			}
		}

		return true;
	}
}
