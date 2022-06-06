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
			void init() override;
			void render(SpriteRenderer &) const override;

		private:
			ItemEntity(const ItemStack &);

			ItemStack stack;

			static std::unordered_map<ItemID, Texture> itemTextureMap;
			static Texture missing;
	};

	void to_json(nlohmann::json &, const ItemEntity &);
}
