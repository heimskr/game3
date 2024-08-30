#pragma once

#include "graphics/Color.h"
#include "graphics/OpenGL.h"
#include "types/Types.h"

namespace Game3 {
	struct RenderOptions {
		double x = 0.;
		double y = 0.;
		double offsetX = 0.;
		double offsetY = 0.;
		double sizeX = 16.;
		double sizeY = 16.;
		double scaleX = 1.;
		double scaleY = 1.;
		double angle = 0.;
		Color color{1., 1., 1., 1.};
		bool invertY = true;
		double viewportX = -1.;
		double viewportY = -1.;
		GLint wrapMode = GL_CLAMP_TO_EDGE;
	};
}
