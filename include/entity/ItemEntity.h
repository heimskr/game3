#pragma once

#include <nlohmann/json.hpp>

#include "Texture.h"
#include "entity/Entity.h"
#include "game/Item.h"

namespace Game3 {
	class ItemEntity: public Entity {
		public:

			const ItemStack & getStack() const { return stack; }
			void setStack(const ItemStack &);

			static std::shared_ptr<ItemEntity> create(const ItemStack &);
			static std::shared_ptr<ItemEntity> fromJSON(const nlohmann::json &);

			nlohmann::json toJSON() const override;

			// void render(SpriteRenderer &) const override;

		private:
			ItemEntity(const ItemStack &);
			static std::unordered_map<EntityID, Texture> itemTextureMap;

			Texture *texture = nullptr;
			ItemStack stack;
	};

	void to_json(nlohmann::json &, const ItemEntity &);
}
