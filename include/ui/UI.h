#pragma once

namespace Game3 {
	class Window;

	class UI {
		public:
			virtual ~UI() = default;

			virtual void render(Window &) = 0;
	};
}
