#pragma once

#include <memory>
#include <utility>

#include "types/Position.h"
#include "types/Types.h"

namespace Game3 {
	class Realm;

	bool simpleAStar(const std::shared_ptr<Realm> &realm, const Position &from, const Position &to, std::vector<Position> &path, size_t loop_max = 1'000);
}
