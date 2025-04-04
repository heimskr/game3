#pragma once

#include "graphics/Color.h"
#include "ui/gl/widget/Widget.h"

#include <optional>

namespace Game3 {
	class Scroller: public Widget {
		public:
			Scroller(UIContext &, float selfScale, Color scrollbar_color);
			Scroller(UIContext &, float selfScale);

			using Widget::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;

			bool click(int button, int x, int y, Modifiers) final;
			bool dragStart(int x, int y) final;
			bool dragUpdate(int x, int y) final;
			bool dragEnd(int x, int y) final;
			bool scroll(float x_delta, float y_delta, int x, int y, Modifiers) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			void clearChildren() final;
			bool onChildrenUpdated() final;
			void childResized(const WidgetPtr &, int, int) final;

			void setChild(WidgetPtr);

		private:
			Color scrollbarColor;
			std::optional<float> lastChildWidth;
			std::optional<float> lastChildHeight;
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
			float fixXOffset(float) const;
			float fixYOffset(float) const;
			void updateHorizontalRectangle();
			void updateVerticalRectangle();
			void maybeRemeasureChildWidth();
			void maybeRemeasureChildHeight();

		friend class ChatDialog;
	};
}
