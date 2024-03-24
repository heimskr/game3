#pragma once

#include <vector>

namespace Game3 {
	template <typename T>
	class RectangularVector: public std::vector<T> {
		public:
			size_t width;
			size_t height;

			RectangularVector(size_t width_, size_t height_) {
				resize(width_, height_, 0);
			}

			RectangularVector(size_t size_): RectangularVector(size_, size_) {}

			void resize(size_t width_, size_t height_, T value) {
				width = width_;
				height = height_;
				std::vector<T>::resize(width * height, value);
			}

			using std::vector<T>::operator[];

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
