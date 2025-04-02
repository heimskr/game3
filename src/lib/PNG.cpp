#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "lib/PNG.h"

namespace Game3 {
	std::shared_ptr<uint8_t[]> getRaw(const png::image<png::rgba_pixel> &image) {
		uint32_t width = image.get_width();
		uint32_t height = image.get_height();
		auto raw = std::make_shared<uint8_t[]>(4 * width * height);
		size_t i = 0;
		for (uint32_t y = 0; y < height; ++y) {
			for (uint32_t x = 0; x < width; ++x) {
				png::rgba_pixel pixel = image.get_pixel(x, y);
				static_assert(sizeof(pixel) == 4, "RGBA pixels must be 4 bytes in size");
				std::memcpy(&raw[i], &pixel, sizeof(pixel));
				i += sizeof(pixel);
			}
		}
		return raw;
	}
}
