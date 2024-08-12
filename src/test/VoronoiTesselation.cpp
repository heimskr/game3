#include "Log.h"
#include "algorithm/Voronoi.h"
#include "util/Math.h"

#include "lib/stb/stb_image.h"
#include "lib/stb/stb_image_write.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <utility>

namespace Game3 {
	namespace {
		constexpr int64_t CHUNK_SIZE = 64;
	}

	void voronoiTest() {
		std::default_random_engine rng(std::random_device{}());

		int count = 16;

		std::uniform_real_distribution point_distribution(0.f, 64.f);

		std::vector<uint32_t> colors{
			0x0a4f75ff,
			0xce7d3eff,
			0xffe13eff,
			0xe2a795ff,
			0xc9eb4aff,
			0x045f5aff,
			0x6feb4cff,
			0xa516bcff,
			0x300e49ff,
			0x59fae1ff,
			0xb74917ff,
			0x060ea1ff,
			0x039ae9ff,
			0xa67fbeff,
			0xfda380ff,
			0xc9bf9bff,
			0x8e66d7ff,
			0x0cd0d8ff,
			0x2016b8ff,
			0xb49904ff,
		};

		std::uniform_int_distribution<size_t> color_distribution(0, colors.size() - 1);

		auto generator = [&] {
			return colors.at(color_distribution(rng));
		};

		const int width(CHUNK_SIZE);
		const int height(CHUNK_SIZE);

		RectangularVector<uint32_t> grid(width, height);

		voronoize<uint32_t>(grid, count, rng, generator);

		std::unique_ptr<uint8_t[]> raw = std::make_unique<uint8_t[]>(width * height * 4);

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				const auto i = 4 * (y * width + x);
				const auto color = grid[x, y];
				for (int p = 0; p < 4; ++p) {
					raw[i + p] = (color >> (8 * (3 - p))) & 0xff;
				}
			}
		}

		std::stringstream ss;
		stbi_write_png_to_func(+[](void *context, void *data, int size) {
			std::stringstream &ss = *reinterpret_cast<std::stringstream *>(context);
			ss << std::string_view(reinterpret_cast<const char *>(data), size);
		}, &ss, width, height, 4, raw.get(), width * 4);
		std::cout << ss.str();
	}
}
