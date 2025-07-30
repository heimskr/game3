#pragma once

#include "graphics/Color.h"
#include "ui/widget/Widget.h"

#include <optional>

namespace Game3 {
	class Scroller: public Widget {
		public:
			Scroller(UIContext &, float selfScale, Color scrollbar_color);
			Scroller(UIContext &, float selfScale);

			using Widget::render;
			void render(const RendererContext &, float x, float y, float width, float height) override;

			bool click(int button, int x, int y, Modifiers) override;
			bool dragStart(int x, int y) override;
			bool dragUpdate(int x, int y) override;
			bool dragEnd(int x, int y, double) override;
			bool scroll(float x_delta, float y_delta, int x, int y, Modifiers) override;

			SizeRequestMode getRequestMode() const override;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) override;

			void clearChildren() override;
			bool onChildrenUpdated() override;
			void childResized(const WidgetPtr &, Orientation, int, int) override;

			void setChild(WidgetPtr);
			float getVerticalOffset() const;
			float getHorizontalOffset() const;
			void scrollToTop();

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
