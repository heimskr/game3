#include "item/ItemStack.h"
#include "item/Tool.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	void Tool::initStack(const Game &, ItemStack &stack) {
		if (!stack.data->contains("durability"))
			(*stack.data)["durability"] = std::make_pair(maxDurability, maxDurability);
	}
}
