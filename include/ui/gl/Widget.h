#pragma once

namespace Game3 {
	class UIContext;
	struct RendererContext;

	class Widget {
		protected:
			Widget() = default;

		public:
			float lastX = -1;
			float lastY = -1;

			virtual ~Widget() = default;

			virtual void render(UIContext &, RendererContext &, float x, float y);
	};
}
