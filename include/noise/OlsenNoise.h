#include <iostream>

#include "noise/NoiseGenerator.h"

namespace Game3 {
	template <typename T = double, typename R = std::mt19937_64>
	class OlsenNoise: public NoiseGenerator<T, R> {
		public:
			ssize_t iterations;

			OlsenNoise(R::result_type seed, size_t side_length, ssize_t iterations_): NoiseGenerator<T, R>(seed, side_length), iterations(iterations_) {}

			~OlsenNoise() override = default;

			decltype(NoiseGenerator<T, R>::output) & run() override {
				return run(T(this->rng()) * 200'000 / this->rng.max() - 100'000, T(this->rng()) * 200'000 / this->rng.max() - 100'000);
			}

			decltype(NoiseGenerator<T, R>::output) & run(T x0, T y0) {
				T x1 = x0 + this->sideLength;
				T y1 = y0 + this->sideLength;

				if (x1 < x0 || y1 < y0 || iterations < 0)
					return this->output;

				this->output.resize(this->sideLength, this->sideLength, 0);

				if (iterations == 0)
					for (size_t x = 0; x < this->sideLength; ++x)
						for (size_t y = 0; y < this->sideLength; ++y)
							this->output(x, y) = hash({size_t(x0 + x), size_t(y0 + y)});

				const auto ux0 = ssize_t(std::floor(x0 / 2)) - 1;
				const auto uy0 = ssize_t(std::floor(y0 / 2)) - 1;
				const auto ux1 = ssize_t(std::floor(x1 / 2)) + 1;
				const auto uy1 = ssize_t(std::floor(y1 / 2)) + 1;

				OlsenNoise<T, R> upper_map(this->seed, ux1 - ux0 + 1, iterations - 1);
				upper_map.run(ux0, uy0);

				const auto uw = ux1 - ux0;
				const auto uh = uy1 - uy0;

				const auto cx0 = ux0 * 2;
				const auto cy0 = uy0 * 2;
				auto cw = uw * 2;
				auto ch = uh * 2;

				const auto upsize = std::max(cw, ch);
				SquareVector<T> upsampled(upsize);

				for (decltype(cw) x = 0; x < cw; ++x)
					for (decltype(ch) y = 0; y < ch; ++y) {
						auto r = hash({size_t(iterations), size_t(cx0 + x), size_t(cy0 + y)}) & ((1 << 7) - iterations);
						upsampled(x, y) = upper_map.output(std::floor(x / 2), std::floor(y / 2)) + r;
					}

				const auto offset_x = x0 - cx0;
				const auto offset_y = y0 - cy0;

				ch -= 2;
				for (decltype(cw) x = 0; x < cw; ++x)
					for (decltype(ch) y = 0; y < ch; ++y)
						upsampled(x, y) = upsampled(x, y) + upsampled(x, y + 1) + upsampled(x, y + 2);

				cw -= 2;
				for (decltype(ch) y = 0; y < ch; ++y)
					for (decltype(cw) x = 0; x < cw; ++x)
						upsampled(x, y) = upsampled(x, y) + upsampled(x + 1, y) + upsampled(x + 2, y);

				for (decltype(this->sideLength) x = 0; x < this->sideLength; ++x)
					for (decltype(this->sideLength) y = 0; y < this->sideLength; ++y)
						this->output(x, y) = std::floor(upsampled(x + offset_x, y + offset_y) / 9);

				return this->output;
			}

			template <typename N>
			N hash(N value) {
				N out = value;
				switch (value & N(3)) {
					case 1:
						out += value;
						out ^= out << N(20);
						out += out >> N(2);
						break;
					case 2:
						out += value;
						out ^= out << N(22);
						out += out >> N(34);
						break;
					case 3:
						out += value;
						out ^= out << N(32);
						out ^= value << N(36);
						out += out >> N(22);
						break;
				}

				out ^= out << N(6);
				out += out >> N(10);
				out ^= out << N(8);
				out += out >> N(34);
				out ^= out << N(50);
				out += out >> N(12);

				return out;
			}

			template <typename N>
			N hash(std::initializer_list<N> values) {
				if (values.size() == 0)
					return 0;
				N out = hash(*values.begin());
				for (auto iter = values.begin() + 1, end = values.end(); iter != end; ++iter)
					out = hash(out ^ *iter);
				return out;
			}

			void normalize(T low, T high) override {
				for (size_t x = 0; x < this->sideLength; ++x)
					for (size_t y = 0; y < this->sideLength; ++y)
						this->output(x, y) = mapNumber(this->output(x, y), T(50), T(196), low, high); // Where the hell did 50 and 196 come from?
			}

		private:

			template <typename N>
			N mapNumber(N value, N old_min, N old_max, N new_min, N new_max) {
				return (value - old_min) * (new_max - new_min) / (old_max - old_min) + new_min;
			}
	};
}
