#pragma once

#include "ui/widget/Widget.h"
#include "ui/HasAlignment.h"
#include "ui/HasFixedSize.h"

#include <optional>

namespace Game3 {
	/** For aligning a single child widget within a fixed-size area. */
	class Aligner: public Widget, public HasFixedSize {
		public:
			Aligner(UIContext &, Orientation, Alignment);

			using Widget::render;
			void render(const RendererContext &, float x, float y, float width, float height) override;

			SizeRequestMode getRequestMode() const override;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) override;

			bool onChildrenUpdated() override;

			void setChild(WidgetPtr);
			void setOrientation(Orientation);
			void setAlignment(Alignment);

		private:
			Orientation orientation;
			Alignment alignment;
			std::optional<float> childSize;

			void markDirty();
	};
}
