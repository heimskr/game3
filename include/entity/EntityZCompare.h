#pragma once

#include <memory>

namespace Game3 {
	class Entity;

	struct EntityZCompare {
		bool operator()(const std::shared_ptr<Entity> &, const std::shared_ptr<Entity> &) const;
	};
}
