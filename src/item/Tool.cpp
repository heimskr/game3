#include "item/Tool.h"

namespace Game3 {
	void Tool::initStack(const Game &, ItemStack &stack) {
		if (!stack.data.contains("durability"))
			stack.data["durability"] = std::make_pair(maxDurability, maxDurability);
	}
}
