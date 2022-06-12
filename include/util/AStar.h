#pragma once

#include <memory>
#include <utility>

#include "Position.h"
#include "Types.h"

namespace Game3 {
	class Realm;

	bool simpleAStar(const std::shared_ptr<Realm> &realm, const Position &from, const Position &to, std::vector<Position> &path);
}
