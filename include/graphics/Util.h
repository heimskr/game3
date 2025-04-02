#pragma once

#include "graphics/RenderOptions.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Game3 {
	class Tileset;
	class Window;
	struct RenderOptions;

	glm::mat4 makeMapModel(const RenderOptions &, int texture_width, int texture_height, const Tileset &, const Window &);
}
