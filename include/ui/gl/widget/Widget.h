#pragma once

#include <memory>

namespace Game3 {
	class UIContext;
	struct Rectangle;
	struct RendererContext;

	class Widget: public std::enable_shared_from_this<Widget> {
		protected:
			int lastX = -1;
			int lastY = -1;
			int lastWidth = -1;
			int lastHeight = -1;

			Widget() = default;

		public:
			Widget(const Widget &) = default;
			Widget(Widget &&) noexcept = default;

			virtual ~Widget() = default;

			Widget & operator=(const Widget &) = default;
			Widget & operator=(Widget &&) noexcept = default;

			virtual Rectangle getLastRectangle() const;
			/** The implementation is free to ignore the `width` and `height` parameters. */
			virtual void render(UIContext &, RendererContext &, float x, float y, float width, float height);
			virtual void render(UIContext &, RendererContext &, const Rectangle &);
			/** Can return a pointer to nothing, itself or a new widget. */
			virtual std::shared_ptr<Widget> getDragStartWidget();
			/** `x` and `y` are absolute, not relative to the top left corner of the widget. */
			virtual bool click(UIContext &, int x, int y);
			virtual bool dragStart(UIContext &, int x, int y);
			virtual void dragUpdate(UIContext &, int x, int y);
			virtual bool dragEnd(UIContext &, int x, int y);
			virtual float calculateHeight(RendererContext &, float available_width, float available_height) = 0;
	};

	using WidgetPtr = std::shared_ptr<Widget>;
}