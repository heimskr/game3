#pragma once

#include <gtkmm.h>

namespace Game3 {
	struct Modifiers {
		bool shift = false;
		bool ctrl  = false;
		bool alt   = false;
		bool super = false;

		Modifiers() = default;
		Modifiers(bool shift_, bool ctrl_, bool alt_, bool super_);
		explicit Modifiers(Gdk::ModifierType);

		inline bool onlyShift() const { return shift && !ctrl && !alt && !super; }
		inline bool onlyCtrl()  const { return !shift && ctrl && !alt && !super; }
		inline bool onlyAlt()   const { return !shift && !ctrl && alt && !super; }
		inline bool onlySuper() const { return !shift && !ctrl && !alt && super; }
	};
}
