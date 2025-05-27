#pragma once

#include <cmath>
#include <concepts>
#include <limits>
#include <utility>

namespace Game3 {
	template <typename T>
	struct Range {
		T left;
		T right;
		bool openLeft;
		bool openRight;

		Range(T left_value, T right_value, bool open_left, bool open_right):
			left(left_value),
			right(right_value),
			openLeft(open_left),
			openRight(open_right) {
				if (left > right) {
					std::swap(left, right);
					std::swap(openLeft, openRight);
				}
			}

		template <std::invocable<T> F>
		requires (!std::predicate<F, T>)
		void iterate(F &&function) {
			T value{};

			if (!openLeft) {
				if constexpr (std::floating_point<T>) {
					value = std::nextafter(left, std::numeric_limits<T>::infinity());
				} else if constexpr (std::integral<T>) {
					value = left + 1;
				} else {
					value = left;
					++value;
				}
			}

			if (openRight) {
				for (; value <= right; ++value) {
					function(value);
				}
			} else {
				for (; value < right; ++value) {
					function(value);
				}
			}
		}

		/** Terminates early and returns true if the predicate ever returns true. */
		template <std::predicate<T> F>
		bool iterate(F &&function) {
			T value{};

			if (!openLeft) {
				if constexpr (std::floating_point<T>) {
					value = std::nextafter(left, std::numeric_limits<T>::infinity());
				} else if constexpr (std::integral<T>) {
					value = left + 1;
				} else {
					value = left;
					++value;
				}
			}

			if (openRight) {
				for (; value <= right; ++value) {
					if (function(value)) {
						return true;
					}
				}
			} else {
				for (; value < right; ++value) {
					if (function(value)) {
						return true;
					}
				}
			}

			return false;
		}

		static auto openOpen(T left, T right) {
			return Range<T>(left, right, true, true);
		}

		static auto openClosed(T left, T right) {
			return Range<T>(left, right, true, false);
		}

		static auto closedOpen(T left, T right) {
			return Range<T>(left, right, false, true);
		}

		static auto closedClosed(T left, T right) {
			return Range<T>(left, right, false, false);
		}
	};
}
