#include "ui/gl/widget/Grid.h"

namespace Game3 {
	void Grid::render(const RendererContext &renderers, float x, float y, float width, float height) {
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
					widget->render(renderers, x, y, child_width, child_height);
					x += child_width + columnSpacing;
				}
			}

			if (max_height > 0)
				y += max_height + rowSpacing;
		}
	}

	SizeRequestMode Grid::getRequestMode() const {
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

	void Grid::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		if (widgetContainer.columns() == 0 || widgetContainer.rows() == 0) {
			minimum = natural = 0;
			return;
		}

		const float for_size = orientation == Orientation::Horizontal? for_width : for_height;
		float accumulated_minimum = 0;
		float accumulated_natural = 0;
		float accumulated_nonexpanding_natural = 0;

		std::size_t outer_size{}, inner_size{};
		std::size_t outer{}, inner{};
		std::size_t *row{}, *column{};
		std::optional<std::vector<float>> *sizes{};

		if (orientation == Orientation::Horizontal) {
			outer_size = widgetContainer.columns();
			inner_size = widgetContainer.rows();
			column = &outer;
			row = &inner;
			sizes = &columnWidths;
		} else {
			outer_size = widgetContainer.rows();
			inner_size = widgetContainer.columns();
			row = &outer;
			column = &inner;
			sizes = &rowHeights;
		}

		if (!*sizes)
			sizes->emplace();

		(*sizes)->resize(outer_size);

		std::size_t expand_count = 0;

		auto get_size_container = [&] -> float & {
			if (orientation == Orientation::Horizontal)
				return sizeContainer[*row, *column].first;
			return sizeContainer[*row, *column].second;
		};

		for (outer = 0; outer < outer_size; ++outer) {
			float max_minimum = 0;
			float max_natural = 0;
			bool expands = false;

			for (inner = 0; inner < inner_size; ++inner) {
				float child_minimum{};
				float child_natural{};

				if (Widget *child = widgetContainer[*row, *column]) {
					child->measure(renderers, orientation, -1, -1, child_minimum, child_natural);
					max_minimum = std::max(max_minimum, child_minimum);
					max_natural = std::max(max_natural, child_natural);
					expands = expands || child->getExpand(orientation);
				}
			}

			if (expands) {
				(*sizes)->at(outer) = -1;
				++expand_count;
			} else {
				(*sizes)->at(outer) = max_natural;
				accumulated_nonexpanding_natural += max_natural;
			}

			for (inner = 0; inner < inner_size; ++inner)
				get_size_container() = max_natural;

			accumulated_minimum += max_minimum;
			accumulated_natural += max_natural;
		}

		if (expand_count > 0) {
			for (outer = 0; outer < outer_size; ++outer) {
				float &size = (*sizes)->at(outer);
				if (size == -1) {
					size = (for_size - accumulated_nonexpanding_natural) / expand_count;
					for (inner = 0; inner < inner_size; ++inner)
						get_size_container() = size;
				}
			}
		}

		const float spacing = (orientation == Orientation::Horizontal? columnSpacing : rowSpacing) * (inner_size - 1);
		minimum = spacing + accumulated_minimum;
		natural = spacing + accumulated_natural;
	}

	void Grid::remove(WidgetPtr child) {
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

		throw std::runtime_error("Couldn't find child in Grid's widgetContainer");

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

	void Grid::clearChildren() {
		Widget::clearChildren();
		widgetContainer.clear();
		markDirty();
	}

	void Grid::attach(WidgetPtr child, std::size_t row, std::size_t column) {
		child->insertAtEnd(shared_from_this());
		detach(row, column);
		widgetContainer[row, column] = child.get();
		markDirty();
	}

	void Grid::detach(std::size_t row, std::size_t column) {
		if (widgetContainer.rows() <= row || widgetContainer.columns() <= column)
			return;

		Widget *&widget = widgetContainer.at(row, column);
		if (widget) {
			remove(widget->shared_from_this());
			widget = nullptr;
			markDirty();
		}
	}

	float Grid::getRowSpacing() const {
		return rowSpacing;
	}

	float Grid::getColumnSpacing() const {
		return columnSpacing;
	}

	void Grid::setRowSpacing(float new_spacing) {
		rowSpacing = new_spacing;
	}

	void Grid::setColumnSpacing(float new_spacing) {
		columnSpacing = new_spacing;
	}

	void Grid::setSpacing(float row_spacing, float column_spacing) {
		setRowSpacing(row_spacing);
		setColumnSpacing(column_spacing);
	}

	void Grid::setSpacing(float spacing) {
		setSpacing(spacing, spacing);
	}

	WidgetPtr Grid::operator[](std::size_t row, std::size_t column) const {
		if (widgetContainer.rows() <= row || widgetContainer.columns() < column)
			return {};

		if (Widget *widget = widgetContainer.at(row, column))
			return widget->shared_from_this();

		return {};
	}

	void Grid::markDirty() {
		sizesDirty = true;
		lastMode.reset();
	}
}
