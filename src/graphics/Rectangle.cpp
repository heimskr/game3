#include "graphics/GL.h"
#include "graphics/Rectangle.h"

namespace Game3 {
	void Rectangle::scissor() const {
		glScissor(x, y, static_cast<GLsizei>(width), static_cast<GLsizei>(height)); CHECKGL
		glViewport(x, y, static_cast<GLsizei>(width), static_cast<GLsizei>(height)); CHECKGL
	}
}
