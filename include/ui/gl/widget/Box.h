#pragma once

#include "graphics/Color.h"
#include "ui/gl/widget/ChildDependentExpandingWidget.h"
#include "ui/gl/widget/Widget.h"
#include "ui/gl/Types.h"

namespace Game3 {
	class Box: public ChildDependentExpandingWidget<Widget> {
		public:
			static constexpr Color DEFAULT_SEPARATOR_COLOR{"#49120060"};

			Box(UIContext &, float selfScale, Orientation, float padding, float separator_thickness, Color separator_color);
			Box(UIContext &, float selfScale, Orientation, float padding, float separator_thickness);
			Box(UIContext &, float selfScale, Orientation, float padding);
			Box(UIContext &, float selfScale, Orientation = Orientation::Vertical);

			using ChildDependentExpandingWidget<Widget>::render;
			void render(const RendererContext &, float x, float y, float width, float height) override;

			SizeRequestMode getRequestMode() const override;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) override;

			bool onChildrenUpdated() override;

			void append(const WidgetPtr &);

			void setPadding(float);
			void setSeparatorThickness(float);
			void setSeparatorColor(Color);

		private:
			Orientation orientation{};
			float padding;
			float separatorThickness;
			Color separatorColor;
			std::vector<std::pair<float, float>> childMeasurements;
			/** If the box is horizontal, this will be the maximum child height; otherwise, it will be the maximum child width. */
			std::optional<float> maximumPerpendicularChildMeasurement;
	};
}
