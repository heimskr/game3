/*
#include "Log.h"
#include "ui/gl/widget/ChildDependentExpandingWidget.h"

namespace Game3 {
	bool ChildDependentExpandingWidget::getHorizontalExpand() const {
		if (horizontalExpand) {
			INFO("\x1b[31m[{}]\x1b[39m", __LINE__);
			return true;
		}

		if (lastCalculatedHorizontalExpand) {
			INFO("\x1b[31m[{}]\x1b[39m -> {}", __LINE__, *lastCalculatedHorizontalExpand);
			return *lastCalculatedHorizontalExpand;
		}

		INFO("\x1b[31m[{}]\x1b[39m", __LINE__);
		return recalculateExpand(Orientation::Horizontal);
	}

	bool ChildDependentExpandingWidget::getVerticalExpand() const {
		if (verticalExpand)
			return true;

		if (lastCalculatedVerticalExpand)
			return *lastCalculatedVerticalExpand;

		return recalculateExpand(Orientation::Vertical);
	}

	bool ChildDependentExpandingWidget::recalculateExpand(Orientation orientation) const {
		INFO("\x1b[33mChecking children of Grid for {} expansion\x1b[39m", orientation);

		std::optional<bool> &cached = orientation == Orientation::Horizontal? lastCalculatedHorizontalExpand : lastCalculatedVerticalExpand;

		if (orientation == Orientation::Horizontal? horizontalExpand : verticalExpand) {
			cached = true;
			return true;
		}

		bool expands = false;

		std::size_t count = 0;

		for (WidgetPtr child = firstChild; child; child = child->getNextSibling()) {
			++count;
			if (child->getExpand(orientation)) {
				expands = true;
				break;
			}
		}

		INFO("\x1b[33mResult: {}\x1b[39m, checked {} child(ren)", expands, count);
		cached = expands;
		return expands;
	}

	void ChildDependentExpandingWidget::markExpansionDirty() const {
		lastCalculatedHorizontalExpand.reset();
		lastCalculatedVerticalExpand.reset();
	}

	bool ChildDependentExpandingWidget::onChildrenUpdated() {
		INFO("ChildDependentExpandingWidget::onChildrenUpdated()");
		if (!Widget::onChildrenUpdated()) {
			INFO("...no");
			return false;
		}

		static int _ = 0;
		INFO("...yes. {}", _); if (_ == 6) raise(SIGTRAP);
		markExpansionDirty();
		++_;
		return true;
	}
}
// */
