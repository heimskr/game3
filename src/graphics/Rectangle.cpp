#include "graphics/GL.h"
#include "graphics/Rectangle.h"

namespace Game3 {
	void Rectangle::scissor(int outer_height) const {
		// `outer_height - y - height` to transform upper left corner to lower left corner
		glScissor(x, outer_height - y - height, static_cast<GLsizei>(width), static_cast<GLsizei>(height)); CHECKGL
		glViewport(x, outer_height - y - height, static_cast<GLsizei>(width), static_cast<GLsizei>(height)); CHECKGL
	}

	bool Rectangle::contains(int x, int y) const {
		return this->x <= x && x < this->x + width && this->y <= y && y < this->y + height;
	}

	Rectangle Rectangle::operator+(const Rectangle &other) const {
		return {x + other.x, y + other.y, other.width, other.height};
	}
}
