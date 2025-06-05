#include "container/ImageSpan.h"
#include "lib/PNG.h"

namespace Game3 {
	std::vector<uint8_t> pixelsToPNG(std::span<const uint8_t> pixels, size_t width, size_t height, size_t channels) {
		std::vector<uint8_t> out;

		stbi_write_png_to_func(+[](void *context, void *data, int size) {
			auto &out = *reinterpret_cast<std::vector<uint8_t> *>(context);
			auto *chars = reinterpret_cast<const uint8_t *>(data);
			std::span span(chars, size);
			out.insert(out.end(), span.begin(), span.end());
		}, &out, width, height, channels, pixels.data(), width * channels);

		return out;
	}
}
