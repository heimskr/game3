#pragma once

#include <format>
#include <string>

namespace Game3 {
	class BasicBuffer;
	class Buffer;

	struct Modifiers {
		bool shift: 1 = false;
		bool ctrl:  1 = false;
		bool alt:   1 = false;
		bool super: 1 = false;

		Modifiers() = default;
		Modifiers(uint8_t);
		Modifiers(bool shift_, bool ctrl_, bool alt_, bool super_);

		explicit operator uint8_t() const;
		explicit operator std::string() const;

		inline bool onlyShift() const { return  shift && !ctrl && !alt && !super; }
		inline bool onlyCtrl()  const { return !shift &&  ctrl && !alt && !super; }
		inline bool onlyAlt()   const { return !shift && !ctrl &&  alt && !super; }
		inline bool onlySuper() const { return !shift && !ctrl && !alt &&  super; }
		inline bool empty()     const { return !shift && !ctrl && !alt && !super; }

		Modifiers operator|(Modifiers) const;
		bool operator==(Modifiers) const;
	};

	template <typename T>
	T popBuffer(BasicBuffer &);
	template <>
	Modifiers popBuffer<Modifiers>(BasicBuffer &);
	Buffer & operator+=(Buffer &, const Modifiers &);
	Buffer & operator<<(Buffer &, const Modifiers &);
	BasicBuffer & operator>>(BasicBuffer &, Modifiers &);
}

template <>
struct std::formatter<Game3::Modifiers> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &modifiers, auto &ctx) const {
		return std::format_to(ctx.out(), "{}", std::string(modifiers));
	}
};
