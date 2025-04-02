#include "ui/gl/SizeCacher.h"

namespace Game3 {
	void SizeCacher::rescale(float new_scale) {
		if (new_scale != previousScale) {
			cachedSize.reset();
			previousScale = new_scale;
		}
	}

	std::pair<int, int> SizeCacher::getCachedSize(const RendererContext &renderers, bool force) {
		if (!force && cachedSize) {
			return *cachedSize;
		}

		float dummy, width{}, height{};
		measure(renderers, Orientation::Horizontal, -1, -1, dummy, width);
		measure(renderers, Orientation::Vertical, -1, -1, dummy, height);
		return cachedSize.emplace(width, height);
	}

	std::optional<std::pair<int, int>> SizeCacher::getCachedSize() const {
		return cachedSize;
	}
}
