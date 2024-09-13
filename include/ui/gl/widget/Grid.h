#pragma once

#include "container/GridContainer.h"
#include "ui/gl/widget/ChildDependentExpandingWidget.h"
#include "ui/gl/widget/Widget.h"

#include <optional>
#include <vector>

namespace Game3 {
	class Grid: public ChildDependentExpandingWidget<Widget> {
		public:
			using ChildDependentExpandingWidget<Widget>::ChildDependentExpandingWidget;

			using ChildDependentExpandingWidget<Widget>::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			void remove(WidgetPtr child) final;
			void clearChildren() final;

			void attach(WidgetPtr, std::size_t row, std::size_t column);
			void detach(std::size_t row, std::size_t column);

			float getRowSpacing() const;
			float getColumnSpacing() const;

			void setRowSpacing(float);
			void setColumnSpacing(float);
			void setSpacing(float row_spacing, float column_spacing);
			void setSpacing(float spacing);

			WidgetPtr operator[](std::size_t row, std::size_t column) const;

		private:
			float rowSpacing = 0;
			float columnSpacing = 0;
			std::optional<std::vector<float>> rowHeights;
			std::optional<std::vector<float>> columnWidths;
			GridContainer<Widget *> widgetContainer;
			mutable std::optional<SizeRequestMode> lastMode;

			bool sizesDirty = false;
			GridContainer<std::pair<float, float>> sizeContainer;

			void markDirty();
	};
}
