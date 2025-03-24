#pragma once

#include "graphics/Rectangle.h"
#include "types/Types.h"
#include "ui/gl/HasExpand.h"
#include "ui/gl/Types.h"
#include "ui/Modifiers.h"

#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <string>

namespace Game3 {
	class UIContext;
	struct Rectangle;
	struct RendererContext;

	class Widget;
	using WidgetPtr = std::shared_ptr<Widget>;
	using WeakWidgetPtr = std::weak_ptr<Widget>;

	class Widget: public std::enable_shared_from_this<Widget>, public HasExpand {
		public:
			Widget(UIContext &, float selfScale);

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
			virtual bool click(int button, int x, int y, Modifiers);
			virtual bool mouseDown(int button, int x, int y, Modifiers);
			virtual bool mouseUp(int button, int x, int y, Modifiers);
			virtual bool dragStart(int x, int y);
			virtual bool dragUpdate(int x, int y);
			virtual bool dragEnd(int x, int y);
			virtual bool scroll(float x_delta, float y_delta, int x, int y, Modifiers);
			virtual bool keyPressed(uint32_t key, Modifiers, bool is_repeat);
			virtual bool charPressed(uint32_t codepoint, Modifiers);
			virtual SizeRequestMode getRequestMode() const = 0;
			virtual void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) = 0;
			float getScale() const;
			virtual bool isDragging() const;
			virtual void onFocus();
			virtual void onBlur();
			/** Calls horizontal `measure` if `width` isn't the same as the last recorded width.
			 *  Calls vertical `measure` if `height` isn't the same as the last recorded height. */
			virtual void maybeRemeasure(const RendererContext &, int width, int height);
			/** Returns whether the given mouse coordinate is over this widget. */
			virtual bool contains(int x, int y) const;

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

			virtual void setName(std::string);
			virtual const std::string & getName() const;
			virtual std::string describe() const;

			UIContext & getUI();

		protected:
			UIContext &ui;
			float selfScale{};
			std::optional<std::pair<int, int>> dragOrigin;
			Rectangle lastRectangle{-1, -1, -1, -1};
			WidgetPtr firstChild;
			WidgetPtr lastChild;
			WeakWidgetPtr weakParent;
			WeakWidgetPtr previousSibling;
			WidgetPtr nextSibling;
			std::size_t childCount = 0;
			bool suppressChildUpdates = false;
			std::string name;

			std::set<std::string> debugAttributes;

			/** Mouse X and Y coordinates are relative to the top left corner of the widget. Return value indicates whether to stop propagation. */
			std::function<bool(Widget &, int button, int mouse_x, int mouse_y)> onClick;

			/** Mouse X and Y coordinates are relative to the top left corner of the widget. Return value indicates whether to stop propagation. */
			std::function<bool(Widget &, int mouse_x, int mouse_y)> onDragStart;

			/** Mouse X and Y coordinates are relative to the top left corner of the widget. Return value indicates whether to stop propagation. */
			std::function<bool(Widget &, int mouse_x, int mouse_y)> onDragUpdate;

			virtual bool shouldCull() const;

		public:
			/** Returns false if updates are suppressed. */
			virtual bool onChildrenUpdated();

			virtual void setOnClick(decltype(onClick));
			virtual void setOnClick(std::function<bool(Widget &)>);
			virtual void setOnClick(std::function<void(Widget &)>);
			virtual void setOnDragStart(decltype(onDragStart));
			virtual void setOnDragUpdate(decltype(onDragUpdate));

			/** Returns true if this element or any of its ancestors (up to a given limit) has a given attribute. */
			virtual bool findAttributeUp(const std::string &, int depth_limit = INT_MAX) const;
			/** Returns true if this element or any of its descendants (up to a given limit) has a given attribute. */
			virtual bool findAttributeDown(const std::string &, int depth_limit = INT_MAX) const;
			virtual bool hasAttribute(const std::string &) const;
			virtual void bestowAttribute(std::string attribute);

		friend class UIContext;
		friend class GeneInfoModule;
	};

	template <typename T, typename... Args>
	requires std::derived_from<T, Widget>
	std::shared_ptr<T> make(Args &&...args) {
		auto dialog = std::make_shared<T>(std::forward<Args>(args)...);
		dialog->init();
		return dialog;
	}
}
