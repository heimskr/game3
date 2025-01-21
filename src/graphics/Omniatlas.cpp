#include "Log.h"
#include "graphics/Omniatlas.h"
#include "lib/PNG.h"
#include "util/FS.h"

#include <cmath>

namespace Game3 {
	Omniatlas::Omniatlas(const std::filesystem::path &dir) {
		std::unordered_map<std::filesystem::path, png::image<png::rgba_pixel>> item_images, other_images;
		std::size_t tile_count = 0;

		const std::filesystem::path items_dir = "resources/items";

		for (const auto &entry: std::filesystem::recursive_directory_iterator(dir)) {
			const auto &path = entry.path();

			if (path.extension() != ".png") {
				continue;
			}

			auto &images = isSubpath(items_dir, path)? item_images : other_images;
			auto iter = images.try_emplace(path, path.string()).first;
			auto &image = iter->second;
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
				images.erase(iter);
			}
		}

		INFO("Omniatlas image count: {} item / {} other, tile count: {}, side length: {}", item_images.size(), other_images.size(), tile_count, std::ceil(std::sqrt(static_cast<double>(tile_count))) * 16);
	}
}
