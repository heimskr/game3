#pragma once

namespace Game3 {
	struct Place;

	struct InteractionSet {
		/** Returns true iff an interaction happened. */
		virtual bool interact(const Place &) const = 0;
	};

	struct StandardInteractions: InteractionSet {
		bool interact(const Place &) const;
	};
}
