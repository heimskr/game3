#pragma once

#include <nlohmann/json.hpp>

#include "Texture.h"
#include "entity/Entity.h"
#include "game/Item.h"

namespace Game3 {
	struct ItemTexture {
		float x;
		float y;
		Texture *texture;
		ItemTexture() = delete;
		ItemTexture(float x_, float y_, Texture &texture_): x(x_), y(y_), texture(&texture_) {}
	};

	class ItemEntity: public Entity {
		public:
			const ItemStack & getStack() const { return stack; }
			void setStack(const ItemStack &);

			static std::shared_ptr<ItemEntity> create(const ItemStack &);
			static std::shared_ptr<ItemEntity> fromJSON(const nlohmann::json &);

			nlohmann::json toJSON() const override;
			void init() override;
			void render(SpriteRenderer &) const override;
			virtual void onInteractOn    (const std::shared_ptr<Player> &player) { interact(player); }
			virtual void onInteractNextTo(const std::shared_ptr<Player> &player) { interact(player); }

		private:
			ItemEntity(const ItemStack &);
			float xOffset = 0.f;
			float yOffset = 0.f;

			ItemStack stack;

			void interact(const std::shared_ptr<Player> &);

			static std::unordered_map<ItemID, ItemTexture> itemTextureMap;
			static Texture missing;
	};

	void to_json(nlohmann::json &, const ItemEntity &);
}
