#pragma once

#include "types/Types.h"
#include "ui/gl/widget/Widget.h"

namespace Game3 {
	class ProgressBarWidget: public Widget {
		private:
			float scale{};
			Color topInteriorColor;
			Color bottomInteriorColor;
			Color backgroundColor;
			Color topExteriorColor;
			Color bottomExteriorColor;
			float progress{};

		public:
			ProgressBarWidget(float scale, Color interior_color, Color background_color, Color exterior_color, float progress = 0);
			ProgressBarWidget(float scale, Color interior_color, float progress = 0);

			void render(UIContext &, RendererContext &, float x, float y, float width, float height) final;

			void setProgress(float);
	};
}
