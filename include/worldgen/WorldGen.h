#pragma once

#include <functional>
#include <memory>
#include <random>

namespace Game3 {
	class Realm;
	struct Position;

	using BuildingGenerator = std::function<void(const std::shared_ptr<Realm> &, std::default_random_engine &, const std::shared_ptr<Realm> &, const Position &)>;
}
