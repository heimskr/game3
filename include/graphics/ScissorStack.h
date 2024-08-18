#pragma once

#include "graphics/Rectangle.h"

#include <stack>

namespace Game3 {
	class ScissorStack {
		private:
			std::stack<Rectangle> stack;

		public:
			ScissorStack() = default;

			Rectangle getTop() const;
			const Rectangle & pushRelative(const Rectangle &);
	};
}
