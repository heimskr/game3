#pragma once

#include <memory>
#include <random>

#include "Types.h"

namespace Game3 {
	class Realm;

	namespace WorldGen {
		void generateKeep(const std::shared_ptr<Realm> &, std::default_random_engine &, RealmID parent_realm, const Position &entrance);
	}
}
