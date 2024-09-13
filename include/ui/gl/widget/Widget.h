#pragma once

#include "graphics/Rectangle.h"
#include "types/Types.h"
#include "ui/gl/HasExpand.h"
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

	class Widget: public std::enable_shared_from_this<Widget>, public HasExpand {
		public:
			Widget(UIContext &, float scale);

			Widget(const Widget &) = delete;
			Widget(Widget &&) noexcept = delete;

			virtual ~Widget() = default;

			Widget & operator=(const Widget &) = delete;
			Widget & operator=(Widget &&) noexcept = delete;

			virtual void init();
			/** Returns absolute coordinates. */
			virtual const Rectangle & getLastRectangle() const;
			/** `x` and `y` are relative to the top of the scissor stack. The implementation is free to ignore the `width` and `height` parameters. */
			virtual void render(const RendererContext &, float x, float y, float width, float height);
			virtual void render(const RendererContext &, const Rectangle &);
			/** Can return a pointer to nothing, itself or a new widget. */
			virtual WidgetPtr getDragStartWidget();
			/** `x` and `y` are absolute, not relative to the top left corner of the widget. */
			virtual bool click(int button, int x, int y);
			virtual bool dragStart(int x, int y);
			virtual bool dragUpdate(int x, int y);
			virtual bool dragEnd(int x, int y);
			virtual bool scroll(float x_delta, float y_delta, int x, int y);
			virtual bool keyPressed(uint32_t character, Modifiers);
			virtual SizeRequestMode getRequestMode() const = 0;
			virtual void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) = 0;
			virtual float getScale() const;
			virtual bool isDragging() const;
			virtual void onFocus();
			virtual void onBlur();

			WidgetPtr getParent() const;
			WidgetPtr getPreviousSibling() const;
			WidgetPtr getNextSibling() const;
			WidgetPtr getFirstChild() const;
			void insertAfter(WidgetPtr parent, WidgetPtr sibling);
			void insertBefore(WidgetPtr parent, WidgetPtr sibling);
			void insertAtStart(WidgetPtr parent);
			void insertAtEnd(WidgetPtr parent);
			virtual void remove(WidgetPtr child);
			virtual void clearChildren();
			virtual std::size_t getChildCount() const;

			UIContext & getUI();

		protected:
			UIContext &ui;
			float scale{};
			std::optional<std::pair<int, int>> dragOrigin;
			Rectangle lastRectangle{-1, -1, -1, -1};
			WidgetPtr firstChild;
			WidgetPtr lastChild;
			WeakWidgetPtr weakParent;
			WeakWidgetPtr previousSibling;
			WidgetPtr nextSibling;
			std::size_t childCount = 0;

			/** Mouse X and Y coordinates are relative to the top left corner of the widget. Return value indicates whether to stop propagation. */
			std::function<bool(Widget &, int button, int mouse_x, int mouse_y)> onClick;

			/** Mouse X and Y coordinates are relative to the top left corner of the widget. Return value indicates whether to stop propagation. */
			std::function<bool(Widget &, int mouse_x, int mouse_y)> onDragStart;

			/** Mouse X and Y coordinates are relative to the top left corner of the widget. Return value indicates whether to stop propagation. */
			std::function<bool(Widget &, int mouse_x, int mouse_y)> onDragUpdate;

		public:
			virtual void setOnClick(decltype(onClick));
			virtual void setOnDragStart(decltype(onDragStart));
			virtual void setOnDragUpdate(decltype(onDragUpdate));

		friend class UIContext;
		friend class GeneInfoModule;
	};
}
