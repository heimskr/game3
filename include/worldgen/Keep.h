#pragma once

#include <memory>
#include <random>

#include "types/Types.h"

namespace Game3 {
	class Realm;
	struct Position;

	namespace WorldGen {
		void generateKeep(const std::shared_ptr<Realm> &, std::default_random_engine &, RealmID parent_realm, Index width, Index height, const Position &entrance, VillageID village_id);
	}
}
