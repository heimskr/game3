#pragma once

#include <memory>

namespace Game3 {
	class Entity;

	struct EntityZCompare {
		bool operator()(const std::shared_ptr<Entity> &, const std::shared_ptr<Entity> &) const;
		bool operator()(const std::weak_ptr<Entity> &, const std::weak_ptr<Entity> &) const;
	};
}
