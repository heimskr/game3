#pragma once

#include "graphics/Color.h"
#include "types/Types.h"
#include "ui/gl/widget/Widget.h"
#include "ui/gl/HasExpand.h"
#include "ui/gl/HasFixedSize.h"

#include <chrono>
#include <optional>

namespace Game3 {
	// TODO: vertical progress bars

	class ProgressBar: public Widget, public HasFixedSize {
		public:
			ProgressBar(UIContext &, float scale, Color interior_color, Color background_color, Color exterior_color, float progress = 0);
			ProgressBar(UIContext &, float scale, Color interior_color, float progress = 0);
			ProgressBar(UIContext &, float scale, float progress = 0);

			using Widget::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			void setProgress(float);

		private:
			Color topInteriorColor;
			Color bottomInteriorColor;
			Color backgroundColor;
			Color topExteriorColor;
			Color bottomExteriorColor;
			float progress{};
			float oldProgress{};
			float lastReportedProgress = -1;
			std::optional<std::chrono::system_clock::time_point> progressUpdatePoint;

			float getDefaultWidth() const;
			float getDefaultHeight() const;
	};
}
