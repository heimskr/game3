#pragma once

#include <concepts>

namespace Game3 {
	/** Represents a floating point value that has a maximum rate of change and is updated over time. */
	template <std::floating_point T>
	class Gradual {
		public:
			Gradual(T value, T rate, T multiplier = 1):
				value(value),
				rate(std::abs(rate)),
				multiplier(multiplier) {}

			Gradual & operator=(T new_value) {
				accumulator = new_value - value;
				return *this;
			}

			template <typename U>
			Gradual & operator+=(const U &amount) {
				accumulator += amount;
				return *this;
			}

			template <typename U>
			Gradual & operator-=(const U &amount) {
				accumulator -= amount;
				return *this;
			}

			template <typename U>
			Gradual & instantAdd(const U &amount) {
				value += amount;
				return *this;
			}

			template <typename U>
			Gradual & instantSubtract(const U &amount) {
				value -= amount;
				return *this;
			}

			template <typename U>
			Gradual & add(const U &amount, bool instant) {
				if (instant) {
					value += amount;
				} else {
					accumulator += amount;
				}
				return *this;
			}

			template <typename U>
			Gradual & subtract(const U &amount, bool instant) {
				if (instant) {
					value -= amount;
				} else {
					accumulator -= amount;
				}
				return *this;
			}

			void clampMin(T minimum) {
				if (value + accumulator < minimum) {
					accumulator = minimum - value;
				}
			}

			void clampMax(T maximum) {
				if (value + accumulator > maximum) {
					accumulator = maximum - value;
				}
			}

			operator T() const { return value; }

			T getValue() const { return value; }
			void setValue(T new_value) {
				value = new_value;
				accumulator = 0;
			}

			T getAccumulator() const { return accumulator; }

			T getRate() const { return rate; }
			void setRate(T new_rate) { rate = new_rate; }

			T getMultiplier() const { return multiplier; }
			void setMultiplier(T new_multiplier) { multiplier = new_multiplier; }

			T getTarget() const { return value + accumulator; }
			void setTarget(T new_value) {
				accumulator = new_value - (value + accumulator);
			}

			/** `delta` is in seconds. */
			template <typename D>
			void tick(D delta) {
				if (accumulator == 0) {
					return;
				}

				T difference = accumulator;
				if (const T max = rate * multiplier * delta; std::abs(difference) > max) {
					difference = difference < 0? -max : max;
				}

				accumulator -= difference;
				value += difference;
			}

		private:
			T value{};
			T accumulator{};
			/** Maximum rate of change in value per second. Forced to be nonnegative. */
			T rate{};
			T multiplier = 1;
	};

	using GradualFloat = Gradual<float>;
	using GradualDouble = Gradual<double>;
}
