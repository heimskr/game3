#pragma once

#include "graphics/Color.h"
#include "ui/gl/widget/Widget.h"
#include "ui/gl/Types.h"

namespace Game3 {
	class Box: public Widget {
		public:
			Box(float scale, Orientation, float padding, float separator_thickness, Color separator_color);
			Box(float scale, Orientation = Orientation::Vertical);

			using Widget::render;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			void setPadding(float);
			void setSeparatorThickness(float);
			void setSeparatorColor(Color);

		private:
			Orientation orientation{};
			float padding;
			float separatorThickness;
			Color separatorColor;
			std::vector<std::pair<float, float>> childMeasurements;
	};
}
