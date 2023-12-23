#pragma once

#include <vector>

namespace Game3 {
	class Canvas;

	class HasBackbuffer {
		public:
			int backbufferWidth  = -1;
			int backbufferHeight = -1;

			HasBackbuffer() = default;

			virtual void update(const Canvas &);
			virtual void update(int width, int height);

			void pushBackbuffer();
			void popBackbuffer();

		private:
			std::vector<std::pair<int, int>> sizeQueue;
	};
}
