#include "Log.h"
#include "entity/Player.h"
#include "entity/SquareParticle.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "graphics/Tileset.h"
#include "item/FluidGun.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "types/Position.h"

namespace Game3 {
	bool FluidGun::use(Slot, const ItemStackPtr &stack, const Place &place, Modifiers, std::pair<float, float> offsets) {
		GamePtr game = stack->getGame();

		constexpr static double scale = 10.5;
		const Position relative = place.position - place.player->getPosition();
		Vector3 velocity(relative.column + (0.5 - offsets.first), relative.row + (0.5 - offsets.second), 16.0);
		velocity.x *= scale;
		velocity.y *= scale;
		velocity.z /= scale;

		auto fluid = game->getFluid("base:fluid/lava");

		auto entity = SquareParticle::create(game, velocity, 0.25, fluid->color);
		entity->spawning = true;
		entity->setRealm(place.realm);
		entity->offset.z = place.player->getOffset().z;
		place.realm->queueEntityInit(std::move(entity), place.player->getPosition());
		constexpr static float variance = .8;
		place.realm->playSound(place.position, "base:sound/bing_chilling", std::uniform_real_distribution(variance, 1.f / variance)(threadContext.rng));

		return false;
	}

	bool FluidGun::drag(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers) {
		return use(slot, stack, place, modifiers, {0.f, 0.f});
	}
}
