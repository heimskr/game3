#pragma once

#include "graphics/Rectangle.h"
#include "types/Types.h"
#include "ui/Modifiers.h"

#include <memory>

namespace Game3 {
	class UIContext;
	struct Rectangle;
	struct RendererContext;

	class Widget: public std::enable_shared_from_this<Widget> {
		protected:
			float scale{};
			Rectangle lastRectangle{-1, -1, -1, -1};

			Widget(float scale);

		public:
			Widget(const Widget &) = default;
			Widget(Widget &&) noexcept = default;

			virtual ~Widget() = default;

			Widget & operator=(const Widget &) = default;
			Widget & operator=(Widget &&) noexcept = default;

			/** Returns absolute coordinates. */
			virtual const Rectangle & getLastRectangle() const;
			/** `x` and `y` are relative to the top of the scissor stack. The implementation is free to ignore the `width` and `height` parameters. */
			virtual void render(UIContext &, const RendererContext &, float x, float y, float width, float height);
			virtual void render(UIContext &, const RendererContext &, const Rectangle &);
			/** Can return a pointer to nothing, itself or a new widget. */
			virtual std::shared_ptr<Widget> getDragStartWidget();
			/** `x` and `y` are absolute, not relative to the top left corner of the widget. */
			virtual bool click(UIContext &, int button, int x, int y);
			virtual bool dragStart(UIContext &, int x, int y);
			virtual bool dragUpdate(UIContext &, int x, int y);
			virtual bool dragEnd(UIContext &, int x, int y);
			virtual bool scroll(UIContext &, float x_delta, float y_delta, int x, int y);
			virtual bool keyPressed(UIContext &, uint32_t character, Modifiers);
			virtual float calculateHeight(const RendererContext &, float available_width, float available_height) = 0;
	};

	using WidgetPtr = std::shared_ptr<Widget>;
}
