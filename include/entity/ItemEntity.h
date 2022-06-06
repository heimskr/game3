#pragma once

#include "entity/Entity.h"
#include "game/Item.h"

namespace Game3 {
	class ItemEntity: public Entity {
		public:
			ItemStack stack;

			ItemEntity(const ItemStack &stack_); 

			nlohmann::json toJSON() const override;
	};
}
