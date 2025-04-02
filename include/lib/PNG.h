#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include "pngpp/png.hpp"
#pragma GCC diagnostic pop

#include "lib/stb/stb_image.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include "lib/stb/stb_image_write.h"
#pragma GCC diagnostic pop

#include <cstdint>
#include <memory>

namespace Game3 {
	std::shared_ptr<uint8_t[]> getRaw(const png::image<png::rgba_pixel> &);
}
