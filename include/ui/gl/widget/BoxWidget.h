#pragma once

#include "graphics/Color.h"
#include "ui/gl/widget/Widget.h"
#include "ui/gl/Types.h"

namespace Game3 {
	class BoxWidget: public Widget {
		public:
			BoxWidget(float scale, Orientation, float padding, float separator_thickness, Color separator_color);
			BoxWidget(float scale, Orientation = Orientation::Vertical);

			using Widget::render;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			std::pair<float, float> calculateSize(const RendererContext &, float available_width, float available_height) final;

		private:
			Orientation orientation{};
			float padding;
			float separatorThickness;
			Color separatorColor;
			std::pair<float, float> lastRenderedSize{-1, -1};
	};
}
