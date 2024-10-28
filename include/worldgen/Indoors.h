#pragma once

#include <memory>
#include <random>

#include "types/Types.h"
#include "types/Position.h"

namespace Game3 {
	class Realm;

	namespace WorldGen {
		/** Generates floors, walls and a door. Returns the position of the door. */
		Position generateIndoors(const std::shared_ptr<Realm> &realm, std::default_random_engine &, const std::shared_ptr<Realm> &parent_realm, Index width, Index height, const Position &entrance, Index door_x,
			const Identifier &wall = "base:tile/wooden_wall", const Identifier &floor = "base:tile/floor");
	}
}
