#pragma once

#include "types/Types.h"

namespace Game3 {
	struct RenderOptions {
		double x = 0.f;
		double y = 0.f;
		double offsetX = 0.f;
		double offsetY = 0.f;
		double sizeX = 16.f;
		double sizeY = 16.f;
		double scaleX = 1.f;
		double scaleY = 1.f;
		double angle = 0.f;
		Color color{1.f, 1.f, 1.f, 1.f};
		bool invertY = true;
	};
}
