#pragma once

#include "graphics/Color.h"
#include "ui/gl/widget/Widget.h"

#include <optional>

namespace Game3 {
	class Scroller: public Widget {
		public:
			Scroller(float scale, Color scrollbar_color);
			Scroller(float scale);

			using Widget::render;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;

			bool click(UIContext &, int button, int x, int y) final;
			bool dragStart(UIContext &, int x, int y) final;
			bool dragUpdate(UIContext &, int x, int y) final;
			bool dragEnd(UIContext &, int x, int y) final;
			bool scroll(UIContext &, float x_delta, float y_delta, int x, int y) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			void setChild(WidgetPtr);

		private:
			Color scrollbarColor;
			float lastChildHeight = -1;
			float xOffset = 0;
			float yOffset = 0;
			bool reverseScroll = false;
			std::optional<int> lastVerticalScrollMouse;
			std::optional<int> lastHorizontalScrollMouse;
			std::optional<Rectangle> lastVerticalScrollbarRectangle;
			std::optional<Rectangle> lastHorizontalScrollbarRectangle;

			bool getNatural() const;
			float getBarThickness() const;
			float getVerticalOffset() const;
			float getHorizontalOffset() const;
			float recalculateYOffset(float vertical_offset) const;
			float recalculateXOffset(float horizontal_offset) const;
			float fixYOffset(float) const;
			void updateVerticalRectangle();
	};
}
