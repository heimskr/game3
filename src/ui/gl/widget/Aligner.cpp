#include "ui/gl/widget/Aligner.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "util/Log.h"

#include <cassert>

namespace Game3 {
	Aligner::Aligner(UIContext &ui, Orientation orientation, Alignment alignment):
		Widget(ui, 1),
		orientation(orientation),
		alignment(alignment) {}

	void Aligner::render(const RendererContext &renderers, float x, float y, float width, float height) {
		fixSizes(width, height);

		Widget::render(renderers, x, y, width, height);

		if (!firstChild) {
			Widget::render(renderers, x, y, width, height);
			return;
		}

		if (!childSize) {
			float dummy{};
			firstChild->measure(renderers, orientation, width, height, childSize.emplace(), dummy);
		}

		const float horizontal = orientation == Orientation::Horizontal;
		const float original_width = width;
		const float original_height = height;

		if (horizontal) {
			width = *childSize;
		} else {
			height = *childSize;
		}


		if (alignment == Alignment::Start) {
			firstChild->render(renderers, x, y, width, height);
		} else if (alignment == Alignment::Center) {
			if (horizontal) {
				firstChild->render(renderers, x + (original_width - width) / 2, y, width, height);
			} else {
				firstChild->render(renderers, x, y + (original_height - height) / 2, width, height);
			}
		} else if (alignment == Alignment::End) {
			if (horizontal) {
				firstChild->render(renderers, x + original_width - width, y, width, height);
			} else {
				firstChild->render(renderers, x, y + original_height - height, width, height);
			}
		} else {
			assert(!"Alignment is valid");
		}
	}

	SizeRequestMode Aligner::getRequestMode() const {
		return SizeRequestMode::ConstantSize;
	}

	void Aligner::measure(const RendererContext &renderers, Orientation measure_orientation, float for_width, float for_height, float &minimum, float &natural) {
		if (measure_orientation == Orientation::Horizontal) {
			Expansion expansion = getHorizontalExpand();

			if (expansion == Expansion::Expand) {
				minimum = fixedWidth < 0? for_width : std::min(for_width, fixedWidth * ui.scale);
				natural = for_width;
				return;
			}

			if (expansion == Expansion::None) {
				minimum = natural = fixedWidth < 0? for_width : fixedWidth * ui.scale;
				return;
			}
		} else {
			Expansion expansion = getVerticalExpand();

			if (expansion == Expansion::Expand) {
				minimum = fixedHeight < 0? for_height : std::min(for_height, fixedHeight * ui.scale);
				natural = for_height;
				return;
			}

			if (expansion == Expansion::None) {
				minimum = natural = fixedHeight < 0? for_height : fixedHeight * ui.scale;
				return;
			}
		}

		// Shrink: forward request to child
		if (firstChild) {
			firstChild->measure(renderers, measure_orientation, for_width, for_height, minimum, natural);
		} else {
			minimum = natural = 0;
		}
	}

	bool Aligner::onChildrenUpdated() {
		if (!Widget::onChildrenUpdated()) {
			return false;
		}

		markDirty();
		return true;
	}

	void Aligner::setChild(WidgetPtr child) {
		clearChildren();
		child->insertAtEnd(shared_from_this());
		markDirty();
	}

	void Aligner::setOrientation(Orientation new_orientation) {
		orientation = new_orientation;
		markDirty();
	}

	void Aligner::setAlignment(Alignment new_alignment) {
		alignment = new_alignment;
		markDirty();
	}

	void Aligner::markDirty() {
		childSize.reset();
	}
}
