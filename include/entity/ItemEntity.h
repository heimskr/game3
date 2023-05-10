#pragma once

#include <nlohmann/json.hpp>

#include "Texture.h"
#include "entity/Entity.h"
#include "item/Item.h"

namespace Game3 {
	class Game;

	class ItemEntity: public Entity {
		public:
			static Identifier ID() { return {"base", "entity/item"}; }
			const ItemStack & getStack() const { return stack; }
			void setStack(ItemStack);

			static std::shared_ptr<ItemEntity> create(Game &, const ItemStack &);
			static std::shared_ptr<ItemEntity> fromJSON(Game &, const nlohmann::json &);

			void toJSON(nlohmann::json &) const override;
			void init(Game &) override;
			void render(SpriteRenderer &) override;
			virtual bool onInteractOn    (const std::shared_ptr<Player> &player) override { return interact(player); }
			virtual bool onInteractNextTo(const std::shared_ptr<Player> &player) override { return interact(player); }
			Glib::ustring getName() override;
			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		private:
			ItemEntity(const Game &, const nlohmann::json &);
			ItemEntity(ItemStack);
			float xOffset = 0.f;
			float yOffset = 0.f;
			bool needsTexture = false;

			ItemStack stack;

			void setTexture(const Game &);
			bool interact(const std::shared_ptr<Player> &);

			static Texture missing;
	};

	void to_json(nlohmann::json &, const ItemEntity &);
}
