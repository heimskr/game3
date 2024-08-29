#pragma once

#include "graphics/Rectangle.h"

#include <vector>

namespace Game3 {
	class ScissorStack {
		private:
			Rectangle base;
			std::vector<Rectangle> stack;

		public:
			ScissorStack();

			inline const Rectangle & getBase() const { return base; }
			void setBase(const Rectangle &);
			Rectangle getTop() const;
			const Rectangle & pushRelative(const Rectangle &);
			const Rectangle & pushAbsolute(const Rectangle &);
			void pop();
			void debug() const;
	};
}
