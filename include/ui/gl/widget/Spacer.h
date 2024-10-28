#pragma once

#include "types/UString.h"
#include "ui/gl/widget/Widget.h"

namespace Game3 {
	class Spacer: public Widget {
		public:
			Spacer(UIContext &, Orientation);

			void init() override;

			using Widget::render;
			void render(const RendererContext &, float x, float y, float width, float height) override;

			SizeRequestMode getRequestMode() const override;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) override;

		protected:
			Orientation orientation{};
	};
}
