#pragma once

#include "graphics/Color.h"

#include <functional>

namespace Game3 {
	class Entity;

	void spawnSquares(Entity &, size_t count, std::function<Color()> &&color_function, double linger_time);
}
