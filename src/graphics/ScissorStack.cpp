#include "Log.h"
#include "graphics/GL.h"
#include "graphics/ScissorStack.h"

#include <algorithm>

namespace Game3 {
	void ScissorStack::Item::apply(int base_height) const {
		if (doViewport)
			rectangle.viewport(base_height);
		if (doScissor)
			rectangle.scissor(base_height);
	}

	ScissorStack::ScissorStack() = default;

	void ScissorStack::setBase(const Rectangle &new_base) {
		base = new_base;
	}

	auto ScissorStack::getTop() const -> Item {
		return stack.empty()? Item(getBase(), true, true) : stack.back();
	}

	const Rectangle & ScissorStack::pushRelative(const Item &item) {
		const Rectangle old = getTop().rectangle;
		const Rectangle &rect = item.rectangle;
		const Item &out = stack.emplace_back(Rectangle(old.x + rect.x, old.y + rect.y, std::min(rect.width, old.width - rect.x), std::min(rect.height, old.height - rect.y)), item.doViewport, item.doScissor);
		out.apply(base.height);
		return out.rectangle;
	}

	const Rectangle & ScissorStack::pushAbsolute(const Item &item) {
		const Item &out = stack.emplace_back(item);
		out.apply(base.height);
		return out.rectangle;
	}

	void ScissorStack::pop() {
		if (!stack.empty())
			stack.pop_back();

		getTop().apply(base.height);
	}

	void ScissorStack::debug() const {
		INFO("{}", getBase());
		for (const Item &item: stack)
			INFO(" -> {}, viewport={}, scissor={}", item.rectangle, item.doViewport, item.doScissor);
	}
}
