#pragma once

#include <ostream>
#include <string>

#include <gtkmm.h>

namespace Game3 {
	class Buffer;

	struct Modifiers {
		bool shift = false;
		bool ctrl  = false;
		bool alt   = false;
		bool super = false;

		Modifiers() = default;
		Modifiers(uint8_t);
		Modifiers(bool shift_, bool ctrl_, bool alt_, bool super_);
		explicit Modifiers(Gdk::ModifierType);

		explicit operator uint8_t() const;
		explicit operator std::string() const;

		inline bool onlyShift() const { return shift && !ctrl && !alt && !super; }
		inline bool onlyCtrl()  const { return !shift && ctrl && !alt && !super; }
		inline bool onlyAlt()   const { return !shift && !ctrl && alt && !super; }
		inline bool onlySuper() const { return !shift && !ctrl && !alt && super; }
	};

	std::ostream & operator<<(std::ostream &, Modifiers);

	template <typename T>
	T popBuffer(Buffer &);
	template <>
	Modifiers popBuffer<Modifiers>(Buffer &);
	Buffer & operator+=(Buffer &, const Modifiers &);
	Buffer & operator<<(Buffer &, const Modifiers &);
	Buffer & operator>>(Buffer &, Modifiers &);
}
