#include "graphics/Rectangle.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "ui/gl/widget/Widget.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "util/Defer.h"
#include "util/Demangle.h"

#include <cassert>

namespace Game3 {
	Widget::Widget(UIContext &ui, float scale):
		ui(ui), scale(scale) {}

	void Widget::init() {}

	const Rectangle & Widget::getLastRectangle() const {
		return lastRectangle;
	}

	void Widget::render(const RendererContext &, float x, float y, float width, float height) {
		lastRectangle = ui.scissorStack.getTop().rectangle + Rectangle(x, y, width, height);
	}

	void Widget::render(const RendererContext &renderers, const Rectangle &rectangle) {
		render(renderers,
		       static_cast<float>(rectangle.x),
		       static_cast<float>(rectangle.y),
		       static_cast<float>(rectangle.width),
		       static_cast<float>(rectangle.height));
	}

	std::shared_ptr<Widget> Widget::getDragStartWidget() {
		return nullptr;
	}

	bool Widget::click(int button, int x, int y) {
		if ((!dragOrigin || *dragOrigin == std::pair{x, y}) && onClick) {
			return onClick(*this, button, x - lastRectangle.x, y - lastRectangle.y);
		}

		for (WidgetPtr child = firstChild; child; child = child->nextSibling) {
			if (child->contains(x, y) && child->click(button, x, y)) {
				return true;
			}
		}

		return false;
	}

	bool Widget::mouseDown(int button, int x, int y) {
		for (WidgetPtr child = firstChild; child; child = child->nextSibling) {
			if (child->contains(x, y) && child->mouseDown(button, x, y)) {
				return true;
			}
		}

		return false;
	}

	bool Widget::mouseUp(int button, int x, int y) {
		for (WidgetPtr child = firstChild; child; child = child->nextSibling) {
			if (child->contains(x, y) && child->mouseUp(button, x, y)) {
				return true;
			}
		}

		return false;
	}

	bool Widget::dragStart(int x, int y) {
		dragOrigin.emplace(x, y);

		if (onDragStart && onDragStart(*this, x - lastRectangle.x, y - lastRectangle.y))
			return true;

		for (WidgetPtr child = firstChild; child; child = child->nextSibling) {
			if (child->contains(x, y) && child->dragStart(x, y)) {
				return true;
			}
		}

		return false;
	}

	bool Widget::dragUpdate(int x, int y) {
		if (onDragUpdate && onDragUpdate(*this, x - lastRectangle.x, y - lastRectangle.y))
			return true;

		for (WidgetPtr child = firstChild; child; child = child->nextSibling) {
			if (child->contains(x, y) && child->dragUpdate(x, y)) {
				return true;
			}
		}

		return false;
	}

	bool Widget::dragEnd(int x, int y) {
		dragOrigin.reset();

		for (WidgetPtr child = firstChild; child; child = child->nextSibling) {
			if (child->contains(x, y) && child->dragEnd(x, y)) {
				return true;
			}
		}

		return false;
	}

	bool Widget::scroll(float x_delta, float y_delta, int x, int y) {
		for (WidgetPtr child = firstChild; child; child = child->nextSibling) {
			if (child->contains(x, y) && child->scroll(x_delta, y_delta, x, y)) {
				return true;
			}
		}

		return false;
	}

	bool Widget::keyPressed(uint32_t, Modifiers, bool) {
		return false;
	}

	bool Widget::charPressed(uint32_t, Modifiers) {
		return false;
	}

	float Widget::getScale() const {
		return scale;
	}

	bool Widget::isDragging() const {
		return dragOrigin.has_value();
	}

	void Widget::onFocus() {}

	void Widget::onBlur() {
		for (WidgetPtr child = firstChild; child; child = child->nextSibling) {
			child->onBlur();
		}
	}

	void Widget::maybeRemeasure(const RendererContext &renderers, int width, int height) {
		float minimum{}, natural{};

		if (width != lastRectangle.width) {
			measure(renderers, Orientation::Horizontal, width, height, minimum, natural);
		}

		if (height != lastRectangle.height) {
			measure(renderers, Orientation::Vertical, width, height, minimum, natural);
		}
	}

	bool Widget::contains(int x, int y) const {
		return lastRectangle.contains(x, y);
	}

	WidgetPtr Widget::getParent() const {
		return weakParent.lock();
	}

	WidgetPtr Widget::getPreviousSibling() const {
		return previousSibling.lock();
	}

	WidgetPtr Widget::getNextSibling() const {
		return nextSibling;
	}

	WidgetPtr Widget::getFirstChild() const {
		return firstChild;
	}

	void Widget::insertAfter(WidgetPtr parent, WidgetPtr sibling) {
		assert(parent);
		assert(sibling);

		WidgetPtr self = shared_from_this();

		if (WidgetPtr old_parent = weakParent.lock(); old_parent == parent) {
			old_parent->remove(self);
		} else {
			weakParent = parent;
		}

		auto next = sibling->nextSibling;
		previousSibling = sibling;
		nextSibling = next;
		if (next) {
			next->previousSibling = self;
		}
		sibling->nextSibling = self;

		if (parent->lastChild == sibling) {
			parent->lastChild = self;
		}

		++parent->childCount;

		parent->onChildrenUpdated();
	}

	void Widget::insertBefore(WidgetPtr parent, WidgetPtr sibling) {
		assert(parent);
		assert(sibling);

		WidgetPtr self = shared_from_this();

		if (WidgetPtr old_parent = weakParent.lock(); old_parent == parent) {
			old_parent->remove(self);
		} else {
			weakParent = parent;
		}

		auto previous = sibling->previousSibling.lock();
		previousSibling = previous;
		nextSibling = sibling;
		if (previous) {
			previous->nextSibling = self;
		}
		sibling->previousSibling = self;

		if (parent->firstChild == sibling) {
			parent->firstChild = self;
		}

		++parent->childCount;

		parent->onChildrenUpdated();
	}

	void Widget::insertAtStart(WidgetPtr parent) {
		assert(parent);

		WidgetPtr self = shared_from_this();

		if (WidgetPtr old_parent = weakParent.lock(); old_parent == parent) {
			old_parent->remove(self);
		} else {
			weakParent = parent;
		}

		nextSibling = parent->firstChild;
		if (nextSibling) {
			nextSibling->previousSibling = self;
		}

		parent->firstChild = self;

		if (!parent->lastChild) {
			parent->lastChild = self;
		}

		++parent->childCount;

		parent->onChildrenUpdated();
	}

	void Widget::insertAtEnd(WidgetPtr parent) {
		assert(parent);

		WidgetPtr self = shared_from_this();

		if (WidgetPtr old_parent = weakParent.lock(); old_parent == parent) {
			old_parent->remove(self);
		} else {
			weakParent = parent;
		}

		previousSibling = parent->lastChild;
		if (auto previous = previousSibling.lock()) {
			previous->nextSibling = self;
		}

		parent->lastChild = self;

		if (!parent->firstChild) {
			parent->firstChild = self;
		}

		++parent->childCount;

		parent->onChildrenUpdated();
	}

	void Widget::remove(WidgetPtr child) {
		assert(child);
		assert(child->getParent().get() == this);

		if (firstChild == child) {
			firstChild = child->nextSibling;
		}

		if (lastChild == child) {
			lastChild = child->previousSibling.lock();
		}

		if (auto previous = child->previousSibling.lock()) {
			previous->nextSibling = child->nextSibling;
		}

		if (child->nextSibling) {
			child->nextSibling->previousSibling = child->previousSibling;
		}

		child->weakParent.reset();
		child->previousSibling.reset();
		child->nextSibling.reset();
		--childCount;

		onChildrenUpdated();
	}

	void Widget::clearChildren() {
		{
			suppressChildUpdates = true;
			Defer defer([this] { suppressChildUpdates = false; });
			while (firstChild)
				remove(firstChild);
		}

		onChildrenUpdated();
	}

	std::size_t Widget::getChildCount() const {
		return childCount;
	}

	void Widget::setName(std::string new_name) {
		name = std::move(new_name);
	}

	const std::string & Widget::getName() const {
		return name;
	}

	std::string Widget::describe() const {
		if (name.empty()) {
			return DEMANGLE(*this);
		}

		return std::format("{}(name=\"{}\")", DEMANGLE(*this), name);
	}

	UIContext & Widget::getUI() {
		return ui;
	}

	bool Widget::onChildrenUpdated() {
		return !suppressChildUpdates;
	}

	bool Widget::shouldCull() const {
		return lastRectangle.width <= 0 || lastRectangle.height <= 0;
	}

	bool Widget::findAttributeUp(const std::string &attribute, int depth_limit) const {
		for (std::shared_ptr<const Widget> widget = shared_from_this(); widget && depth_limit >= 0; --depth_limit, widget = widget->getParent()) {
			if (widget->hasAttribute(attribute)) {
				return true;
			}
		}

		return false;
	}

	bool Widget::findAttributeDown(const std::string &attribute, int depth_limit) const {
		if (hasAttribute(attribute)) {
			return true;
		}

		if (depth_limit > 0) {
			for (WidgetPtr child = firstChild; child; child = child->nextSibling) {
				if (child->findAttributeDown(attribute, depth_limit - 1)) {
					return true;
				}
			}
		}

		return false;
	}

	bool Widget::hasAttribute(const std::string &attribute) const {
		return debugAttributes.contains(attribute);
	}

	void Widget::bestowAttribute(std::string attribute) {
		debugAttributes.insert(std::move(attribute));
	}

	void Widget::setOnClick(decltype(onClick) new_onclick) {
		onClick = std::move(new_onclick);
	}

	void Widget::setOnClick(std::function<bool(Widget &)> new_onclick) {
		onClick = [new_onclick = std::move(new_onclick)](Widget &widget, int button, int, int) -> bool {
			return button == LEFT_BUTTON && new_onclick(widget);
		};
	}

	void Widget::setOnClick(std::function<void(Widget &)> new_onclick) {
		onClick = [new_onclick = std::move(new_onclick)](Widget &widget, int button, int, int) -> bool {
			if (button != LEFT_BUTTON) {
				return false;
			}

			new_onclick(widget);
			return true;
		};
	}

	void Widget::setOnDragStart(decltype(onDragStart) new_ondragstart) {
		onDragStart = std::move(new_ondragstart);
	}

	void Widget::setOnDragUpdate(decltype(onDragUpdate) new_ondragupdate) {
		onDragUpdate = std::move(new_ondragupdate);
	}
}
