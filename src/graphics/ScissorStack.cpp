#include "graphics/GL.h"
#include "graphics/ScissorStack.h"

#include <algorithm>

namespace Game3 {
	ScissorStack::ScissorStack() = default;

	void ScissorStack::setBase(const Rectangle &new_base) {
		base = new_base;
	}

	Rectangle ScissorStack::getTop() const {
		return stack.empty()? getBase() : stack.top();
	}

	const Rectangle & ScissorStack::pushRelative(const Rectangle &rect) {
		const Rectangle old = getTop();
		const Rectangle &out = stack.emplace(old.x + rect.x, old.y + rect.y, std::min(rect.width, old.width - rect.x), std::min(rect.height, old.height - rect.y));
		out.scissor();
		return out;
	}
}
