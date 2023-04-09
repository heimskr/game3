#pragma once

namespace Game3 {
	template <typename T>
	struct Range {
		T min;
		T max;
		bool openLeft;
		bool openRight;

		bool contains(const T &item) const {
			return !((item < min || (!openLeft && item == min)) || (max < item || (!openRight && item == max)));
		}

		/** Note: doesn't handle the case where T is iterable and the range is (n, succ(n)), which should be considered unsatisfiable. */
		bool satisfiable() const {
			return (min < max) || (min == max && openLeft && openRight);
		}
	};
}
