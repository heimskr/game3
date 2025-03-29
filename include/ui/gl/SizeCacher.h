#pragma once

#include "ui/gl/Types.h"

namespace Game3 {
	struct RendererContext;

	class SizeCacher {
		public:
			virtual void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) = 0;
			void rescale(float);

		protected:
			std::pair<int, int> getCachedSize(const RendererContext &, bool force = false);
			std::optional<std::pair<int, int>> getCachedSize() const;

		private:
			std::optional<std::pair<int, int>> cachedSize;
			float previousScale = -1;
	};
}
