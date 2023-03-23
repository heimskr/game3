#pragma once

#include <memory>
#include <random>

#include "Types.h"
#include "Position.h"

namespace Game3 {
	class Realm;

	namespace WorldGen {
		/** Generates floors, walls and a door. Returns the index of the door.
		 *  door_pos is a horizontal index, not a combined horizontal and vertical index. */
		Index generateIndoors(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, const std::shared_ptr<Realm> &parent_realm, const Position &entrance, Index door_pos = -1);
	}
}
