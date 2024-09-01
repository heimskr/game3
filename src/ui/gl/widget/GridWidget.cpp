#include "ui/gl/widget/GridWidget.h"

namespace Game3 {
	void GridWidget::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		if (sizesDirty) {
			float dummy{};
			measure(renderers, Orientation::Horizontal, width, height, dummy, dummy);
			measure(renderers, Orientation::Vertical,   width, height, dummy, dummy);
			sizesDirty = false;
		}

		const float original_x = x;

		for (std::size_t row = 0; row < widgetContainer.rows(); ++row) {
			x = original_x;
			float max_height = 0;

			for (std::size_t column = 0; column < widgetContainer.columns(); ++column) {
				if (Widget *widget = widgetContainer[row, column]) {
					const auto [child_width, child_height] = sizeContainer.at(row, column);
					max_height = std::max(max_height, child_height);
					widget->render(ui, renderers, x, y, child_width, child_height);
					x += child_width + columnSpacing;
				}
			}

			if (max_height > 0)
				y += max_height + rowSpacing;
		}
	}

	SizeRequestMode GridWidget::getRequestMode() const {
		// Basically stolen from Gtk4.

		if (lastMode)
			return *lastMode;

		using enum SizeRequestMode;

		std::size_t wfh_count = 0;
		std::size_t hfw_count = 0;

		for (WidgetPtr widget = firstChild; widget; widget = widget->getNextSibling()) {
			switch (widget->getRequestMode()) {
				case WidthForHeight:
					++wfh_count;
					break;

				case HeightForWidth:
					++hfw_count;
					break;

				case ConstantSize:
				case Expansive:
					break;
			}
		}

		if (wfh_count == 0 && hfw_count == 0) {
			lastMode = ConstantSize;
			return ConstantSize;
		}

		lastMode = wfh_count > hfw_count? WidthForHeight : HeightForWidth;
		return *lastMode;
	}

	void GridWidget::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		if (widgetContainer.columns() == 0 || widgetContainer.rows() == 0) {
			minimum = natural = 0;
			return;
		}

		float accumulated_minimum = 0;
		float accumulated_natural = 0;

		for (std::size_t column = 0; column < widgetContainer.columns(); ++column) {
			float max_minimum = 0;
			float max_natural = 0;

			for (std::size_t row = 0; row < widgetContainer.rows(); ++row) {
				float child_minimum{};
				float child_natural{};

				if (Widget *child = widgetContainer[row, column]) {
					child->measure(renderers, orientation, -1, -1, child_minimum, child_natural);
					max_minimum = std::max(max_minimum, child_minimum);
					max_natural = std::max(max_natural, child_natural);
				}
			}

			for (std::size_t row = 0; row < widgetContainer.rows(); ++row) {
				if (orientation == Orientation::Horizontal)
					sizeContainer[row, column].first = max_natural;
				else
					sizeContainer[row, column].second = max_natural;
			}

			accumulated_minimum += max_minimum;
			accumulated_natural += max_natural;
		}

		float spacing = orientation == Orientation::Horizontal? columnSpacing * (widgetContainer.columns() - 1) : rowSpacing * (widgetContainer.rows() - 1);
		minimum = spacing + accumulated_minimum;
		natural = spacing + accumulated_natural;
	}

	void GridWidget::remove(WidgetPtr child) {
		if (child->getParent().get() != this)
			return;

		Widget::remove(child);

		// TODO: change when rowspan and colspan are implemented
		auto row_iter = widgetContainer.begin();
		std::size_t column{};

		for (; row_iter != widgetContainer.end(); ++row_iter) {
			column = 0;

			for (Widget *&widget: *row_iter) {
				if (widget == child.get()) {
					widget = nullptr;
					goto done;
				}

				++column;
			}
		}

		throw std::runtime_error("Couldn't find child in GridWidget's widgetContainer");

	done:
		bool can_erase_row = true;
		for (Widget *&widget: *row_iter) {
			if (widget) {
				can_erase_row = false;
				break;
			}
		}

		if (can_erase_row)
			widgetContainer.eraseRow(row_iter - widgetContainer.begin());

		bool can_erase_column = true;
		for (auto &row: widgetContainer) {
			if (row.at(column)) {
				can_erase_column = false;
				break;
			}
		}

		if (can_erase_column)
			widgetContainer.eraseColumn(column);
	}

	void GridWidget::attach(WidgetPtr child, std::size_t row, std::size_t column) {
		child->insertAtEnd(shared_from_this());
		detach(row, column);
		widgetContainer[row, column] = child.get();
		markDirty();
	}

	void GridWidget::detach(std::size_t row, std::size_t column) {
		if (widgetContainer.rows() <= row || widgetContainer.columns() <= column)
			return;

		Widget *&widget = widgetContainer.at(row, column);
		if (widget) {
			remove(widget->shared_from_this());
			widget = nullptr;
			markDirty();
		}
	}

	float GridWidget::getRowSpacing() const {
		return rowSpacing;
	}

	float GridWidget::getColumnSpacing() const {
		return columnSpacing;
	}

	void GridWidget::setRowSpacing(float new_spacing) {
		rowSpacing = new_spacing;
	}

	void GridWidget::setColumnSpacing(float new_spacing) {
		columnSpacing = new_spacing;
	}

	void GridWidget::setSpacing(float row_spacing, float column_spacing) {
		setRowSpacing(row_spacing);
		setColumnSpacing(column_spacing);
	}

	void GridWidget::setSpacing(float spacing) {
		setSpacing(spacing, spacing);
	}

	WidgetPtr GridWidget::operator[](std::size_t row, std::size_t column) const {
		if (widgetContainer.rows() <= row || widgetContainer.columns() < column)
			return {};

		if (Widget *widget = widgetContainer.at(row, column))
			return widget->shared_from_this();

		return {};
	}

	void GridWidget::markDirty() {
		sizesDirty = true;
		lastMode.reset();
	}
}
