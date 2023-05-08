#include "ui/Modifiers.h"

namespace Game3 {
	Modifiers::Modifiers(bool shift_, bool ctrl_, bool alt_, bool super_):
		shift(shift_),
		ctrl (ctrl_ ),
		alt  (alt_  ),
		super(super_) {}

	Modifiers::Modifiers(Gdk::ModifierType gdk):
		shift(static_cast<int>(gdk & Gdk::ModifierType::SHIFT_MASK  ) != 0),
		ctrl (static_cast<int>(gdk & Gdk::ModifierType::CONTROL_MASK) != 0),
		alt  (static_cast<int>(gdk & Gdk::ModifierType::ALT_MASK    ) != 0),
		super(static_cast<int>(gdk & Gdk::ModifierType::SUPER_MASK  ) != 0) {}
}
