#include "Log.h"
#include "graphics/GL.h"
#include "graphics/Rectangle.h"

namespace Game3 {
	void Rectangle::scissor(int outer_height) const {
		// `outer_height - y - height` to transform upper left corner to lower left corner
		glScissor(x, outer_height - y - height, static_cast<GLsizei>(width), static_cast<GLsizei>(height)); CHECKGL
	}

	void Rectangle::viewport(int outer_height) const {
		// `outer_height - y - height` to transform upper left corner to lower left corner
		glViewport(x, outer_height - y - height, static_cast<GLsizei>(width), static_cast<GLsizei>(height)); CHECKGL
	}

	bool Rectangle::contains(int x, int y) const {
		return this->x <= x && x < this->x + width && this->y <= y && y < this->y + height;
	}

	void Rectangle::reposition(int x, int y) & {
		this->x = x;
		this->y = y;
	}

	Rectangle && Rectangle::reposition(int x, int y) && {
		this->x = x;
		this->y = y;
		return std::move(*this);
	}

	Rectangle Rectangle::operator+(const Rectangle &other) const {
		return {x + other.x, y + other.y, other.width, other.height};
	}

	Rectangle Rectangle::operator-(const Rectangle &other) const {
		return {x - other.x, y - other.y, width, height};
	}
}
