#include "graphics/GL.h"
#include "graphics/ScissorStack.h"

#include <algorithm>

namespace Game3 {
	Rectangle ScissorStack::getTop() const {
		if (stack.empty()) {
			GL::Viewport viewport;
			return {viewport.saved[0], viewport.saved[1], viewport.saved[2], viewport.saved[3]};
		}

		return stack.top();
	}

	const Rectangle & ScissorStack::pushRelative(const Rectangle &rect) {
		const Rectangle old = getTop();
		const Rectangle &out = stack.emplace(old.x + rect.x, old.y + rect.y, std::min(rect.width, old.width - rect.x), std::min(rect.height, old.height - rect.y));
		out.scissor();
		return out;
	}
}
