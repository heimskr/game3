#pragma once

#include "ui/widget/Widget.h"

namespace Game3 {
	// No uses with Base being anything other than Widget. Perhaps move back to being a non-template class?

	template <typename Base>
	class ChildDependentExpandingWidget: public Base {
		public:
			using Base::Base;

			Expansion getHorizontalExpand() const override {
				if (this->horizontalExpand == Expansion::Expand) {
					return Expansion::Expand;
				}

				if (lastCalculatedHorizontalExpand) {
					return *lastCalculatedHorizontalExpand;
				}

				return recalculateExpand(Orientation::Horizontal);
			}

			Expansion getVerticalExpand() const override {
				if (this->verticalExpand == Expansion::Expand) {
					return Expansion::Expand;
				}

				if (lastCalculatedVerticalExpand) {
					return *lastCalculatedVerticalExpand;
				}

				return recalculateExpand(Orientation::Vertical);
			}

		protected:
			mutable std::optional<Expansion> lastCalculatedHorizontalExpand;
			mutable std::optional<Expansion> lastCalculatedVerticalExpand;

			Expansion recalculateExpand(Orientation orientation) const {
				std::optional<Expansion> &cached = orientation == Orientation::Horizontal? lastCalculatedHorizontalExpand : lastCalculatedVerticalExpand;

				if ((orientation == Orientation::Horizontal? this->horizontalExpand : this->verticalExpand) == Expansion::Expand) {
					cached = Expansion::Expand;
					return Expansion::Expand;
				}

				Expansion expands = Expansion::None;

				for (WidgetPtr child = this->firstChild; child; child = child->getNextSibling()) {
					if (child->getExpand(orientation) == Expansion::Expand) {
						expands = Expansion::Expand;
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
