#pragma once

namespace Game3 {
	class UIContext;
	struct RendererContext;

	class Dialog {
		protected:
			Dialog() = default;

		public:
			virtual ~Dialog() = default;

			virtual void render(UIContext &, RendererContext &) = 0;
	};
}
