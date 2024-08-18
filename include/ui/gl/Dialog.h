#pragma once

#include <array>
#include <string_view>

namespace Game3 {
	class UIContext;
	struct RendererContext;

	class Dialog {
		protected:
			Dialog() = default;

		public:
			virtual ~Dialog() = default;

			virtual void render(UIContext &, RendererContext &) = 0;

			/** Order: clockwise starting at top left. */
			void drawFrame(UIContext &, RendererContext &, double scale, bool alpha, const std::array<std::string_view, 8> &, const Color &interior = {0, 0, 0, 0});
	};
}
