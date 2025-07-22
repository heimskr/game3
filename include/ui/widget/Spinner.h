#pragma once

#include "graphics/Color.h"
#include "ui/widget/Widget.h"
#include "ui/HasFixedSize.h"

namespace Game3 {
	class Spinner: public Widget, public HasFixedSize {
		public:
			Spinner(UIContext &, float selfScale, Color = {1, 1, 1, 1}, int count = 8, float radius = 0.75, float speed = 0.75);

			using Widget::render;
			void render(const RendererContext &, float x, float y, float width, float height) override;

			SizeRequestMode getRequestMode() const override;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) override;

		protected:
			Color color;
			int count;
			float radius;
			float speed;
			float rotation{};
	};
}
