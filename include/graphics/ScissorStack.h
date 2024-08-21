#pragma once

#include "graphics/Rectangle.h"

#include <stack>

namespace Game3 {
	class ScissorStack {
		private:
			Rectangle base;
			std::stack<Rectangle> stack;

		public:
			ScissorStack();

			inline const Rectangle & getBase() const { return base; }
			void setBase(const Rectangle &);
			Rectangle getTop() const;
			const Rectangle & pushRelative(const Rectangle &);
	};
}
