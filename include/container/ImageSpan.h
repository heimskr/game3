#pragma once

#include <cassert>
#include <concepts>
#include <cstdint>
#include <span>
#include <vector>

namespace Game3 {
	std::vector<uint8_t> pixelsToPNG(std::span<const uint8_t> pixels, size_t width, size_t height, size_t channels);

	template <bool Const>
	class BaseImageSpan: public std::span<std::conditional_t<Const, const uint8_t, uint8_t>> {
		public:
			using T = std::conditional_t<Const, const uint8_t, uint8_t>;
			size_t width{};
			size_t height{};
			size_t channels{};

			BaseImageSpan(T *data, size_t width, size_t height, size_t channels):
				std::span<T>(data, width * height * channels),
				width(width),
				height(height),
				channels(channels) {}

			using std::span<T>::operator[];

			std::vector<uint8_t> toVector() const {
				return {this->begin(), this->end()};
			}

			std::vector<uint8_t> toPNG() const {
				return pixelsToPNG(*this, width, height, channels);
			}

			size_t stride() const {
				return width * channels;
			}

			template <typename Self>
			auto && gray(this Self &&self, size_t x, size_t y) {
				assert(self.channels == 1);
				return self.operator[]((x + y * self.width) * self.channels);
			}

			template <typename Self>
			auto && red(this Self &&self, size_t x, size_t y) {
				assert(self.channels >= 3);
				return self.operator[]((x + y * self.width) * self.channels);
			}

			template <typename Self>
			auto && green(this Self &&self, size_t x, size_t y) {
				assert(self.channels >= 3);
				return self.operator[]((x + y * self.width) * self.channels + 1);
			}

			template <typename Self>
			auto && blue(this Self &&self, size_t x, size_t y) {
				assert(self.channels >= 3);
				return self.operator[]((x + y * self.width) * self.channels + 2);
			}

			template <typename Self>
			auto && alpha(this Self &&self, size_t x, size_t y) {
				if (self.channels < 4) {
					return 0xff;
				}

				return self.operator[]((x + y * self.width) * self.channels + 3);
			}


	};

	using ImageSpan = BaseImageSpan<false>;
	using ConstImageSpan = BaseImageSpan<true>;
}
