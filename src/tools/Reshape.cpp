#include "container/ImageSpan.h"
#include "lib/PNG.h"
#include "tools/Reshape.h"
#include "util/Util.h"

#include <sstream>

namespace Game3 {
	std::vector<uint8_t> reshape47(std::span<const uint8_t> png_bytes, uint32_t tile_size) {
		int x{}, y{}, channels{};

		auto input = std::unique_ptr<uint8_t[], FreeDeleter>(stbi_load_from_memory(png_bytes.data(), png_bytes.size(), &x, &y, &channels, 4));

		if (!input) {
			throw std::invalid_argument("Couldn't perform reshape47: invalid input");
		}

		if (channels != 4) {
			throw std::runtime_error("Expected 4 channels");
		}

		if (static_cast<uint32_t>(x) != tile_size * 11) {
			throw std::invalid_argument("Invalid image width");
		}

		if (static_cast<uint32_t>(y) != tile_size * 5) {
			throw std::invalid_argument("Invalid image height");
		}

		std::vector<uint8_t> output((tile_size * 8) * (tile_size * 8) * 4);

		ImageSpan source(input.get(), x, y, channels);
		ImageSpan destination(output.data(), tile_size * 8, tile_size * 8, 4);

		auto copy_tile = [&](int from_index, int to_index) {
			const int from_x = tile_size * (from_index % 11);
			const int from_y = tile_size * (from_index / 11);
			const int to_x = tile_size * (to_index % 8);
			const int to_y = tile_size * (to_index / 8);
			const uint8_t *from = &source.red(from_x, from_y);
			uint8_t *to = &destination.red(to_x, to_y);
			const size_t from_stride = source.stride();
			const size_t to_stride = destination.stride();
			for (uint32_t row = 0; row < tile_size; ++row) {
				std::memmove(to, from, tile_size * channels);
				from += from_stride;
				to += to_stride;
			}
		};

		for (int index = 0; index < 47; ++index) {
			copy_tile(index, index);
		}

		return destination.toPNG();
	}

	std::vector<uint8_t> reshape47(const std::string &png_bytes, uint32_t tile_size) {
		return reshape47(std::span{reinterpret_cast<const uint8_t *>(png_bytes.data()), png_bytes.size()}, tile_size);
	}
}
