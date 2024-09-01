#pragma once

#include "graphics/Rectangle.h"
#include "types/Types.h"
#include "ui/gl/Types.h"
#include "ui/Modifiers.h"

#include <functional>
#include <memory>

namespace Game3 {
	class UIContext;
	struct Rectangle;
	struct RendererContext;

	class Widget;
	using WidgetPtr = std::shared_ptr<Widget>;
	using WeakWidgetPtr = std::weak_ptr<Widget>;

	class Widget: public std::enable_shared_from_this<Widget> {
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
			virtual WidgetPtr getDragStartWidget();
			/** `x` and `y` are absolute, not relative to the top left corner of the widget. */
			virtual bool click(UIContext &, int button, int x, int y);
			virtual bool dragStart(UIContext &, int x, int y);
			virtual bool dragUpdate(UIContext &, int x, int y);
			virtual bool dragEnd(UIContext &, int x, int y);
			virtual bool scroll(UIContext &, float x_delta, float y_delta, int x, int y);
			virtual bool keyPressed(UIContext &, uint32_t character, Modifiers);
			virtual SizeRequestMode getRequestMode() const = 0;
			virtual void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) = 0;
			virtual float getScale() const;
			virtual bool isDragging() const;

			WidgetPtr getParent() const;
			WidgetPtr getPreviousSibling() const;
			WidgetPtr getNextSibling() const;
			void insertAfter(WidgetPtr parent, WidgetPtr sibling);
			void insertBefore(WidgetPtr parent, WidgetPtr sibling);
			void insertAtStart(WidgetPtr parent);
			void insertAtEnd(WidgetPtr parent);
			void remove(WidgetPtr child);

		protected:
			float scale{};
			bool dragging = false;
			Rectangle lastRectangle{-1, -1, -1, -1};
			WidgetPtr firstChild;
			WidgetPtr lastChild;
			WeakWidgetPtr weakParent;
			WeakWidgetPtr previousSibling;
			WidgetPtr nextSibling;
			size_t childCount = 0;

			/** Mouse X and Y coordinates are relative to the top left corner of the widget. Return value indicates whether to stop propagation. */
			std::function<bool(Widget &, UIContext &, int button, int mouse_x, int mouse_y)> onClick;

			/** Mouse X and Y coordinates are relative to the top left corner of the widget. Return value indicates whether to stop propagation. */
			std::function<bool(Widget &, UIContext &, int mouse_x, int mouse_y)> onDragStart;

			/** Mouse X and Y coordinates are relative to the top left corner of the widget. Return value indicates whether to stop propagation. */
			std::function<bool(Widget &, UIContext &, int mouse_x, int mouse_y)> onDragUpdate;

			Widget(float scale);

		public:
			virtual void setOnClick(decltype(onClick));
			virtual void setOnDragStart(decltype(onDragStart));
			virtual void setOnDragUpdate(decltype(onDragUpdate));

		friend class UIContext;
	};
}
