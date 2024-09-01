#include "Log.h"
#include "graphics/Rectangle.h"
#include "ui/gl/widget/Widget.h"
#include "ui/gl/UIContext.h"
#include "util/Demangle.h"

#include <cassert>

namespace Game3 {
	Widget::Widget(float scale):
		scale(scale) {}

	const Rectangle & Widget::getLastRectangle() const {
		return lastRectangle;
	}

	void Widget::render(UIContext &ui, const RendererContext &, float x, float y, float width, float height) {
		lastRectangle = ui.scissorStack.getTop().rectangle + Rectangle(x, y, width, height);
	}

	void Widget::render(UIContext &ui, const RendererContext &renderers, const Rectangle &rectangle) {
		render(ui, renderers,
		       static_cast<float>(rectangle.x),
		       static_cast<float>(rectangle.y),
		       static_cast<float>(rectangle.width),
		       static_cast<float>(rectangle.height));
	}

	std::shared_ptr<Widget> Widget::getDragStartWidget() {
		return nullptr;
	}

	bool Widget::click(UIContext &ui, int button, int x, int y) {
		if (onClick)
			return onClick(*this, ui, button, x - lastRectangle.x, y - lastRectangle.y);

		for (WidgetPtr child = firstChild; child; child = child->nextSibling)
			if (child->getLastRectangle().contains(x, y) && child->click(ui, button, x, y))
				return true;

		return false;
	}

	bool Widget::dragStart(UIContext &ui, int x, int y) {
		dragging = true;

		if (onDragStart && onDragStart(*this, ui, x - lastRectangle.x, y - lastRectangle.y))
			return true;

		for (WidgetPtr child = firstChild; child; child = child->nextSibling)
			if (child->getLastRectangle().contains(x, y) && child->dragStart(ui, x, y))
				return true;

		return false;
	}

	bool Widget::dragUpdate(UIContext &ui, int x, int y) {
		if (onDragUpdate && onDragUpdate(*this, ui, x - lastRectangle.x, y - lastRectangle.y))
			return true;

		for (WidgetPtr child = firstChild; child; child = child->nextSibling)
			if (child->getLastRectangle().contains(x, y) && child->dragUpdate(ui, x, y))
				return true;

		return false;
	}

	bool Widget::dragEnd(UIContext &ui, int x, int y) {
		dragging = false;

		for (WidgetPtr child = firstChild; child; child = child->nextSibling)
			if (child->getLastRectangle().contains(x, y) && child->dragEnd(ui, x, y))
				return true;

		return false;
	}

	bool Widget::scroll(UIContext &ui, float x_delta, float y_delta, int x, int y) {
		for (WidgetPtr child = firstChild; child; child = child->nextSibling)
			if (child->getLastRectangle().contains(x, y) && child->scroll(ui, x_delta, y_delta, x, y))
				return true;

		return false;
	}

	bool Widget::keyPressed(UIContext &, uint32_t, Modifiers) {
		return false;
	}

	float Widget::getScale() const {
		return scale;
	}

	bool Widget::isDragging() const {
		return dragging;
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

	void Widget::insertAfter(WidgetPtr parent, WidgetPtr sibling) {
		assert(parent);
		assert(sibling);

		WidgetPtr self = shared_from_this();

		if (WidgetPtr old_parent = weakParent.lock(); old_parent == parent)
			old_parent->remove(self);
		else
			weakParent = parent;

		auto next = sibling->nextSibling;
		previousSibling = sibling;
		nextSibling = next;
		if (next)
			next->previousSibling = self;
		sibling->nextSibling = self;

		if (parent->lastChild == sibling)
			parent->lastChild = self;

		++childCount;
	}

	void Widget::insertBefore(WidgetPtr parent, WidgetPtr sibling) {
		assert(parent);
		assert(sibling);

		WidgetPtr self = shared_from_this();

		if (WidgetPtr old_parent = weakParent.lock(); old_parent == parent)
			old_parent->remove(self);
		else
			weakParent = parent;

		auto previous = sibling->previousSibling.lock();
		previousSibling = previous;
		nextSibling = sibling;
		if (previous)
			previous->nextSibling = self;
		sibling->previousSibling = self;

		if (parent->firstChild == sibling)
			parent->firstChild = self;

		++childCount;
	}

	void Widget::insertAtStart(WidgetPtr parent) {
		assert(parent);

		WidgetPtr self = shared_from_this();

		if (WidgetPtr old_parent = weakParent.lock(); old_parent == parent)
			old_parent->remove(self);
		else
			weakParent = parent;

		nextSibling = parent->firstChild;
		if (nextSibling)
			nextSibling->previousSibling = self;

		parent->firstChild = self;

		if (!parent->lastChild)
			parent->lastChild = self;

		++childCount;
	}

	void Widget::insertAtEnd(WidgetPtr parent) {
		assert(parent);

		WidgetPtr self = shared_from_this();

		if (WidgetPtr old_parent = weakParent.lock(); old_parent == parent)
			old_parent->remove(self);
		else
			weakParent = parent;

		previousSibling = parent->lastChild;
		if (auto previous = previousSibling.lock())
			previous->nextSibling = self;

		parent->lastChild = self;

		if (!parent->firstChild)
			parent->firstChild = self;

		++childCount;
	}

	void Widget::remove(WidgetPtr child) {
		assert(child);
		assert(child->getParent().get() == this);

		if (firstChild == child)
			firstChild = child->nextSibling;

		if (auto previous = child->previousSibling.lock())
			previous->nextSibling = child->nextSibling;

		if (child->nextSibling)
			child->nextSibling->previousSibling = child->previousSibling;

		child->weakParent.reset();
		--childCount;
	}

	void Widget::setOnClick(decltype(onClick) new_onclick) {
		onClick = std::move(new_onclick);
	}

	void Widget::setOnDragStart(decltype(onDragStart) new_ondragstart) {
		onDragStart = std::move(new_ondragstart);
	}

	void Widget::setOnDragUpdate(decltype(onDragUpdate) new_ondragupdate) {
		onDragUpdate = std::move(new_ondragupdate);
	}
}
