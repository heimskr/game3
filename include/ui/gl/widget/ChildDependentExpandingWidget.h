#pragma once

#include "ui/gl/widget/Widget.h"

namespace Game3 {
	// No uses with Base being anything other than Widget. Perhaps move back to being a non-template class?

	template <typename Base>
	class ChildDependentExpandingWidget: public Base {
		public:
			using Base::Base;

			bool getHorizontalExpand() const override {
				if (this->horizontalExpand) {
					return true;
				}

				if (lastCalculatedHorizontalExpand) {
					return *lastCalculatedHorizontalExpand;
				}

				return recalculateExpand(Orientation::Horizontal);
			}

			bool getVerticalExpand() const override {
				if (this->verticalExpand) {
					return true;
				}

				if (lastCalculatedVerticalExpand) {
					return *lastCalculatedVerticalExpand;
				}

				return recalculateExpand(Orientation::Vertical);
			}

		protected:
			mutable std::optional<bool> lastCalculatedHorizontalExpand;
			mutable std::optional<bool> lastCalculatedVerticalExpand;

			bool recalculateExpand(Orientation orientation) const {
				std::optional<bool> &cached = orientation == Orientation::Horizontal? lastCalculatedHorizontalExpand : lastCalculatedVerticalExpand;

				if (orientation == Orientation::Horizontal? this->horizontalExpand : this->verticalExpand) {
					cached = true;
					return true;
				}

				bool expands = false;

				for (WidgetPtr child = this->firstChild; child; child = child->getNextSibling()) {
					if (child->getExpand(orientation)) {
						expands = true;
						break;
					}
				}

				cached = expands;
				return expands;
			}

			void markExpansionDirty() const {
				lastCalculatedHorizontalExpand.reset();
				lastCalculatedVerticalExpand.reset();
			}

			bool onChildrenUpdated() override {
				if (!Base::onChildrenUpdated()) {
					return false;
				}

				markExpansionDirty();
				return true;
			}
	};
}
