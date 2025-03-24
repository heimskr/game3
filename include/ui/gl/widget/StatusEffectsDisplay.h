#pragma once

#include "ui/gl/widget/Widget.h"

namespace Game3 {
	/** An overlay for status effect icons. Aligns icons at the top right corner. Ignores all events. */
	class StatusEffectsDisplay: public Widget {
		public:
			StatusEffectsDisplay(UIContext &, float selfScale);

			using Widget::render;
			void render(const RendererContext &, float x, float y, float width, float height) override;

			SizeRequestMode getRequestMode() const override;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) override;
	};
}
