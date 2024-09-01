#pragma once

#include <concepts>
#include <deque>

namespace Game3 {
	template <typename T, template <typename...> typename C = std::deque>
	requires std::default_initializable<T>
	class GridContainer {
		public:
			inline T & at(std::size_t row, std::size_t column) {
				return _rows.at(row).at(column);
			}

			inline const T & at(std::size_t row, std::size_t column) const {
				return _rows.at(row).at(column);
			}

			inline C<T> & rowAt(std::size_t row) {
				return _rows.at(row);
			}

			inline const C<T> & rowAt(std::size_t row) const {
				return _rows.at(row);
			}

			void insertRow(std::size_t index) {
				_rows.emplace(_rows.begin() + index);
				++rowCount;
			}

			void insertColumn(std::size_t index) {
				for (C<T> &row: _rows)
					row.emplace(row.begin() + index);
				++columnCount;
			}

			void eraseRow(std::size_t index) {
				_rows.erase(_rows.begin() + index);
				--rowCount;
			}

			void eraseColumn(std::size_t index) {
				for (C<T> &row: _rows)
					row.erase(row.begin() + index);
				--columnCount;
			}

			inline T & operator[](std::size_t row, std::size_t column) {
				return _rows[row][column];
			}

			inline const T & operator[](std::size_t row, std::size_t column) const {
				return _rows[row][column];
			}

			inline T & operator[](std::size_t row) {
				return _rows[row];
			}

			inline const T & operator[](std::size_t row) const {
				return _rows[row];
			}

			inline std::size_t rows() const {
				return rowCount;
			}

			inline std::size_t columns() const {
				return columnCount;
			}

			inline C<C<T>> & data() {
				return _rows;
			}

			inline const C<C<T>> & data() const {
				return _rows;
			}

		private:
			C<C<T>> _rows;
			std::size_t rowCount = 0;
			std::size_t columnCount = 0;
	};
}
