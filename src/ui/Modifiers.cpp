#include "net/Buffer.h"
#include "ui/Modifiers.h"

namespace Game3 {
	Modifiers::Modifiers(uint8_t bits):
		shift((bits >> 0) & 1),
		ctrl ((bits >> 1) & 1),
		alt  ((bits >> 2) & 1),
		super((bits >> 3) & 1) {}

	Modifiers::Modifiers(bool shift_, bool ctrl_, bool alt_, bool super_):
		shift(shift_),
		ctrl (ctrl_ ),
		alt  (alt_  ),
		super(super_) {}

#ifndef GAME3_SERVER_ONLY
	Modifiers::Modifiers(Gdk::ModifierType gdk):
		shift(static_cast<int>(gdk & Gdk::ModifierType::SHIFT_MASK  ) != 0),
		ctrl (static_cast<int>(gdk & Gdk::ModifierType::CONTROL_MASK) != 0),
		alt  (static_cast<int>(gdk & Gdk::ModifierType::ALT_MASK    ) != 0),
		super(static_cast<int>(gdk & Gdk::ModifierType::SUPER_MASK  ) != 0) {}
#endif

	Modifiers::operator uint8_t() const {
		return (shift? 1 : 0) | (ctrl? 2 : 0) | (alt? 4 : 0) | (super? 8 : 0);
	}

	Modifiers::operator std::string() const {
		std::string out;
		if (shift)
			out += 'S';
		if (ctrl)
			out += 'C';
		if (alt)
			out += 'A';
		if (super)
			out += 'M';
		if (out.empty())
			out = '_';
		return out;
	}

	Modifiers Modifiers::operator|(Modifiers other) const {
		return {
			shift || other.shift,
			ctrl  || other.ctrl,
			alt   || other.alt,
			super || other.super
		};
	}

	bool Modifiers::operator==(Modifiers other) const {
		return shift == other.shift && ctrl == other.ctrl && alt == other.alt && super == other.super;
	}

	template <>
	std::string Buffer::getType(const Modifiers &, bool) {
		return getType(uint8_t{}, false);
	}

	template <>
	Modifiers popBuffer<Modifiers>(Buffer &buffer) {
		return {popBuffer<uint8_t>(buffer)};
	}

	Buffer & operator+=(Buffer &buffer, const Modifiers &modifiers) {
		return buffer += static_cast<uint8_t>(modifiers);
	}

	Buffer & operator<<(Buffer &buffer, const Modifiers &modifiers) {
		return buffer.appendType(modifiers, false) += modifiers;
	}

	Buffer & operator>>(Buffer &buffer, Modifiers &modifiers) {
		const auto type = buffer.popType();
		if (!buffer.typesMatch(type, buffer.getType(modifiers, false))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected Modifiers = u8)");
		}
		modifiers = popBuffer<Modifiers>(buffer);
		return buffer;
	}
}
