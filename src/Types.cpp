#include "net/Buffer.h"
#include "types/Types.h"

namespace Game3 {
	Index operator""_idx(unsigned long long value) {
		return static_cast<Index>(value);
	}

	std::ostream & operator<<(std::ostream &os, PipeType pipe_type) {
		switch (pipe_type) {
			case PipeType::Item:   return os << "Item";
			case PipeType::Fluid:  return os << "Fluid";
			case PipeType::Energy: return os << "Energy";
			default:
				return os << "Invalid";
		}
	}

	std::ostream & operator<<(std::ostream &os, Hand hand) {
		switch (hand) {
			case Hand::None:  return os << "None";
			case Hand::Left:  return os << "Left";
			case Hand::Right: return os << "Right";
			default:
				return os << "Invalid";
		}
	}

	// Color should be moved to its own header/.cpp honestly.

	Color Color::fromBytes(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
		return Color{red / 255.f, green / 255.f, blue / 255.f, alpha / 255.f};
	}

	template <>
	std::string Buffer::getType(const Color &) {
		return std::string{'\x33'} + getType(float{});
	}

	std::ostream & operator<<(std::ostream &stream, const Color &color) {
		return stream << '(' << color.red << ", " << color.green << ", " << color.blue << " @ " << color.alpha << ')';
	}

	Buffer & operator+=(Buffer &buffer, const Color &color) {
		return (((buffer.appendType(color) += color.red) += color.green) += color.blue) += color.alpha;
	}

	Buffer & operator<<(Buffer &buffer, const Color &color) {
		return buffer += color;
	}

	Buffer & operator>>(Buffer &buffer, Color &color) {
		const auto type = buffer.popType();
		if (!Buffer::typesMatch(type, buffer.getType(color))) {
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
