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

			void drawEight(UIContext &, RendererContext &, double scale, bool alpha, const std::array<std::string_view, 8> &);
	};
}
