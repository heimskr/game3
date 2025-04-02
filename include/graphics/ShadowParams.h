#pragma once

namespace Game3 {
	struct ShadowParams {
		double baseX = 0.5;
		double baseY = 1.0 - 1.0 / 8.0;
		double sizeDivisor = 4.0;
		double sizeMinuend = 1.0;
		double sizeClampMin = 0.0;
		double sizeClampMax = 0.8;
		double divisorX = 4.0;
		double divisorY = 8.0;
	};
}
