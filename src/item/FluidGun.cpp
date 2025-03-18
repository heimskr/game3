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
	constexpr static double scale = 2;

	static auto makeParticle(const GamePtr &game, const FluidPtr &fluid, const Place &place, std::pair<float, float> offsets) {
		auto [x_offset, y_offset] = offsets;
		Position relative = place.position - place.player->getPosition();
		Vector3 velocity(relative.column + (0.5 - x_offset), relative.row + (0.5 - y_offset), 16.0);
		velocity.x *= scale;
		velocity.y *= scale;
		velocity.z /= scale;
		auto entity = SquareParticle::create(game, velocity, 0.25, fluid->color);
		entity->spawning = true;
		entity->setRealm(place.realm);
		entity->offset.z = place.player->getOffset().z;
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
			if (diff > 1000) {
				last_play = now;
				constexpr static float variance = .9;
				place.realm->playSound(place.position, "base:sound/bing_chilling", std::uniform_real_distribution(variance, 1.f / variance)(threadContext.rng));
			}
		}

		return true;
	}
}
