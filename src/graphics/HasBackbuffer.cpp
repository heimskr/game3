#include "graphics/HasBackbuffer.h"
#include "graphics/RectangleRenderer.h"
#include "ui/Canvas.h"

#include <stdexcept>

namespace Game3 {
	void HasBackbuffer::update(const Canvas &canvas) {
		update(canvas.getWidth(), canvas.getHeight());
	}

	void HasBackbuffer::update(int width, int height) {
		backbufferWidth  = width;
		backbufferHeight = height;
	}

	void HasBackbuffer::pushBackbuffer() {
		sizeQueue.emplace_back(backbufferWidth, backbufferHeight);
	}

	void HasBackbuffer::popBackbuffer() {
		if (sizeQueue.empty())
			throw std::runtime_error("Backbuffer queue is empty");

		const auto [width, height] = sizeQueue.back();
		sizeQueue.pop_back();
		update(width, height);
	}
}
