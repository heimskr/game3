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

		int to_index = 0;

		auto copy_tiles = [&](int start, int count) {
			for (int i = 0; i < count; ++i) {
				copy_tile(start + i, to_index++);
			}
		};

		// This is the layout of the non-transparent tiles in what Tilesetter produces for Blob tiling.
		copy_tiles(0, 10);
		copy_tiles(11, 10);
		copy_tiles(22, 22);
		copy_tiles(48, 5);

		return destination.toPNG();
	}

	std::vector<uint8_t> reshape47(const std::string &png_bytes, uint32_t tile_size) {
		return reshape47(std::span{reinterpret_cast<const uint8_t *>(png_bytes.data()), png_bytes.size()}, tile_size);
	}

	std::vector<uint8_t> reshapeEntityVariant2(std::span<const uint8_t> png_bytes) {
		constexpr uint32_t cell_size = 16;

		int x{}, y{}, channels{};

		auto input = std::unique_ptr<uint8_t[], FreeDeleter>(stbi_load_from_memory(png_bytes.data(), png_bytes.size(), &x, &y, &channels, 4));

		if (!input) {
			throw std::invalid_argument("Couldn't perform reshapeEntityVariant2: invalid input");
		}

		if (channels != 4) {
			throw std::runtime_error("Expected 4 channels");
		}

		if (static_cast<uint32_t>(x) < cell_size * 9 || x % cell_size != 0) {
			throw std::invalid_argument("Invalid image width");
		}

		if (static_cast<uint32_t>(y) < cell_size * 15 || y % cell_size != 0) {
			throw std::invalid_argument("Invalid image height");
		}

		std::vector<uint8_t> output((cell_size * 8) * (cell_size * 8) * 4);

		ImageSpan source(input.get(), x, y, channels);
		ImageSpan destination(output.data(), cell_size * 8, cell_size * 8, 4);

		for (int i = 0; i < 5 * 8; ++i) {
			const int from_x = 2 * cell_size * (i % 5);
			const int from_y = 2 * cell_size * (i / 5);
			const int to_x = cell_size * (i % 5);
			const int to_y = cell_size * (i / 5);
			const uint8_t *from = &source.red(from_x, from_y);
			uint8_t *to = &destination.red(to_x, to_y);
			const size_t from_stride = source.stride();
			const size_t to_stride = destination.stride();
			for (uint32_t row = 0; row < cell_size; ++row) {
				std::memmove(to, from, cell_size * channels);
				from += from_stride;
				to += to_stride;
			}
		}

		return destination.toPNG();
	}

	std::vector<uint8_t> reshapeEntityVariant2(const std::string &png_bytes) {
		return reshapeEntityVariant2(std::span{reinterpret_cast<const uint8_t *>(png_bytes.data()), png_bytes.size()});
	}
}
