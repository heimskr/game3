#pragma once

#include <ostream>
#include <string>

#ifndef GAME3_SERVER_ONLY
#include <gtkmm.h>
#endif

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

#ifndef GAME3_SERVER_ONLY
		explicit Modifiers(Gdk::ModifierType);
#endif

		explicit operator uint8_t() const;
		explicit operator std::string() const;

		inline bool empty() const { return !shift && !ctrl && !alt && !super; }

		inline bool onlyShift() const { return shift && !ctrl && !alt && !super; }
		inline bool onlyCtrl()  const { return !shift && ctrl && !alt && !super; }
		inline bool onlyAlt()   const { return !shift && !ctrl && alt && !super; }
		inline bool onlySuper() const { return !shift && !ctrl && !alt && super; }

		Modifiers operator|(Modifiers) const;
		bool operator==(Modifiers) const;
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
