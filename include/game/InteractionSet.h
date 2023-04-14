#pragma once

namespace Game3 {
	struct Place;

	struct InteractionSet {
		virtual ~InteractionSet() = default;
		/** Returns true iff an interaction happened. */
		virtual bool interact(const Place &) const = 0;
		virtual bool damageGround(const Place &) const = 0;
	};

	struct StandardInteractions: InteractionSet {
		bool interact(const Place &) const;
		bool damageGround(const Place &) const;
	};
}
