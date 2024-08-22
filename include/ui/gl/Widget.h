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
			virtual ~Widget() = default;

			virtual Rectangle getLastRectangle() const;
			virtual void render(UIContext &, RendererContext &, float x, float y);
			/** Can return a pointer to nothing, itself or a new widget. */
			virtual std::shared_ptr<Widget> getDragStartWidget();
			virtual bool click(UIContext &);
	};

	using WidgetPtr = std::shared_ptr<Widget>;
}
