#include "item/Tool.h"

namespace Game3 {
	void Tool::initStack(const Game &, ItemStack &stack) {
		if (stack.data.is_null()) {
			stack.data.emplace_object();
		}

		auto &object = stack.data.is_null()? stack.data.emplace_object() : stack.data.as_object();

		if (!object.contains("durability")) {
			object["durability"] = boost::json::array{maxDurability, maxDurability};
		}
	}
}
