#include "net/Buffer.h"
#include "types/Types.h"

namespace Game3 {
	Index operator""_idx(unsigned long long value) {
		return static_cast<Index>(value);
	}

	// Color should be moved to its own header/.cpp honestly.

	Color Color::fromBytes(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
		return Color{red / 255.f, green / 255.f, blue / 255.f, alpha / 255.f};
	}

	template <>
	std::string Buffer::getType(const Color &, bool) {
		return std::string{'\x33'} + getType(float{}, false);
	}

	Buffer & operator+=(Buffer &buffer, const Color &color) {
		return (((buffer.appendType(color, false) += color.red) += color.green) += color.blue) += color.alpha;
	}

	Buffer & operator<<(Buffer &buffer, const Color &color) {
		return buffer += color;
	}

	Buffer & operator>>(Buffer &buffer, Color &color) {
		const auto type = buffer.popType();
		if (!Buffer::typesMatch(type, buffer.getType(color, false))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected shortlist<f32, 4> for Color)");
		}
		popBuffer(buffer, color.red);
		popBuffer(buffer, color.green);
		popBuffer(buffer, color.blue);
		popBuffer(buffer, color.alpha);
		return buffer;
	}
}
