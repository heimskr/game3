#pragma once

namespace Game3 {
	class UIContext;
	struct RendererContext;

	class Widget {
		protected:
			Widget() = default;

		public:
			virtual ~Widget() = default;

			virtual void render(UIContext &, RendererContext &, float x, float y) = 0;
	};
}
