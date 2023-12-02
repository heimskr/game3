#include "types/Types.h"

namespace Game3 {
	struct HasMaxDurability {
		Durability maxDurability;

		HasMaxDurability(Durability max_durability):
			maxDurability(max_durability) {}

		virtual ~HasMaxDurability() = default;
	};
}
