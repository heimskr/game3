#pragma once

#include <span>

namespace Game3 {
	template <typename T>
	class RectangularSpan: public std::span<T> {
		public:
			size_t width{};
			size_t height{};

			RectangularVector(T *data, size_t width, size_t height):
				std::span<T>(data, width * height),
				width(width),
				height(height) {}

			using std::span<T>::operator[];
			using std::span<T>::at;

			T & operator[](size_t x, size_t y) {
				return (*this)[x + y * width];
			}

			const T & operator[](size_t x, size_t y) const {
				return (*this)[x + y * width];
			}

			T & at(size_t x, size_t y) {
				return at(x + y * width);
			}

			const T & at(size_t x, size_t y) const {
				return at(x + y * width);
			}
	};
}
