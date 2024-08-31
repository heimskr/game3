#pragma once

#include "graphics/Color.h"
#include "ui/gl/widget/Widget.h"

#include <optional>

namespace Game3 {
	class ScrollerWidget: public Widget {
		public:
			ScrollerWidget(float scale, Color scrollbar_color);
			ScrollerWidget(float scale);

			using Widget::render;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			bool click(UIContext &, int button, int x, int y) final;
			bool dragStart(UIContext &, int x, int y) final;
			bool dragUpdate(UIContext &, int x, int y) final;
			bool dragEnd(UIContext &, int x, int y) final;
			bool scroll(UIContext &, float x_delta, float y_delta, int x, int y) final;
			std::pair<float, float> calculateSize(const RendererContext &, float available_width, float available_height) final;

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
