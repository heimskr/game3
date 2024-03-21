#pragma once

#include "data/Identifier.h"

namespace Game3 {
	/** For things that have a fluid type associated with them, such as filled flasks and mutagen. */
	struct HasFluidType {
		virtual Identifier getFluidType() const = 0;
	};
}
