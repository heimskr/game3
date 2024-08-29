#pragma once

#include "types/Types.h"
#include "ui/gl/widget/Widget.h"

namespace Game3 {
	class ProgressBarWidget: public Widget {
		private:
			float fixedHeight{};
			float scale{};
			Color topInteriorColor;
			Color bottomInteriorColor;
			Color backgroundColor;
			Color topExteriorColor;
			Color bottomExteriorColor;
			float progress{};

		public:
			ProgressBarWidget(float fixed_height, float scale, Color interior_color, Color background_color, Color exterior_color, float progress = 0);
			ProgressBarWidget(float fixed_height, float scale, Color interior_color, float progress = 0);

			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			float calculateHeight(const RendererContext &, float available_width, float available_height) final;

			void setProgress(float);
	};
}
