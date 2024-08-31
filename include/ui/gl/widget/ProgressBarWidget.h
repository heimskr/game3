#pragma once

#include "graphics/Color.h"
#include "types/Types.h"
#include "ui/gl/widget/Widget.h"

#include <chrono>
#include <optional>

namespace Game3 {
	class ProgressBarWidget: public Widget {
		public:
			ProgressBarWidget(float scale, float fixed_height, Color interior_color, Color background_color, Color exterior_color, float progress = 0);
			ProgressBarWidget(float scale, float fixed_height, Color interior_color, float progress = 0);

			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			std::pair<float, float> calculateSize(const RendererContext &, float available_width, float available_height) final;

			void setProgress(float);

		private:
			float fixedHeight{};
			Color topInteriorColor;
			Color bottomInteriorColor;
			Color backgroundColor;
			Color topExteriorColor;
			Color bottomExteriorColor;
			float progress{};
			float oldProgress{};
			float lastReportedProgress = -1;
			std::optional<std::chrono::system_clock::time_point> progressUpdatePoint;
	};
}
