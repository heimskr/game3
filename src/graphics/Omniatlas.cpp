#include "Log.h"
#include "graphics/Omniatlas.h"

#include "pngpp/png.hpp"

#include <cmath>

namespace Game3 {
	Omniatlas::Omniatlas(const std::filesystem::path &dir) {
		std::vector<png::image<png::rgba_pixel>> images;
		std::size_t tile_count = 0;

		for (const auto &entry: std::filesystem::recursive_directory_iterator(dir)) {
			const auto &path = entry.path();
			if (path.extension() == ".png") {
				auto &image = images.emplace_back(path.string());
				bool can_add = true;
				const auto &text_map = image.get_text_map();

				for (const auto &[key, value]: text_map) {
					if (key == "NoAtlas" || (key == "Comment" && value.find("[noatlas]") != std::string::npos)) {
						can_add = false;
						break;
					}
				}

				if (can_add) {
					auto width = image.get_width();
					auto height = image.get_height();

					if (width % 16 != 0 || height % 16 != 0) {
						throw std::runtime_error(std::format("Invalid dimensions encountered in {}: {} x {}", path.string(), width, height));
					}

					tile_count += (width / 16) * (height / 16);
				} else {
					images.pop_back();
				}
			}
		}

		INFO("Omniatlas image count: {}, tile count: {}, side length: {}", images.size(), tile_count, std::ceil(std::sqrt(static_cast<double>(tile_count))) * 16);
	}
}
