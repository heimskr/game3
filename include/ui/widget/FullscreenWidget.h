#pragma once

#include "ui/widget/Widget.h"

namespace Game3 {
	/** Base class for widgets that opt out of layout and render at a fixed position regardless of their parents. */
	class FullscreenWidget: public Widget {
		public:
			using Widget::Widget;

			SizeRequestMode getRequestMode() const override;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) override;
	};
}
