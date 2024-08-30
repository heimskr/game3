#pragma once

#include "ui/gl/widget/Widget.h"

namespace Game3 {
	class ScrollerWidget: public Widget {
		public:
			ScrollerWidget(float scale);

			using Widget::render;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			bool click(UIContext &, int button, int x, int y) final;
			bool dragStart(UIContext &, int x, int y) final;
			bool dragUpdate(UIContext &, int x, int y) final;
			bool dragEnd(UIContext &, int x, int y) final;
			bool scroll(UIContext &, float x_delta, float y_delta, int x, int y) final;
			float calculateHeight(const RendererContext &, float available_width, float available_height) final;

			void setChild(WidgetPtr);

		private:
			float xOffset = 0;
			float yOffset = 0;
			WidgetPtr child;

			bool getNatural() const;
	};
}
