#pragma once

#include "graphics/Texture.h"
#include "entity/Entity.h"
#include "item/Item.h"
#include "item/ItemStack.h"
#include "threading/Atomic.h"

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class Game;

	class ItemEntity: public Entity {
		public:
			static Identifier ID() { return {"base", "entity/item"}; }
			const ItemStack & getStack() const { return stack; }
			void setStack(ItemStack);

			static std::shared_ptr<ItemEntity> create(Game &);
			static std::shared_ptr<ItemEntity> create(Game &, const ItemStack &);
			static std::shared_ptr<ItemEntity> fromJSON(Game &, const nlohmann::json &);

			void toJSON(nlohmann::json &) const override;
			void init(Game &) override;
			void tick(Game &, float) override;
			void render(SpriteRenderer &, TextRenderer &) override;
			virtual bool onInteractOn    (const std::shared_ptr<Player> &player, Modifiers) override { return interact(player); }
			virtual bool onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers) override { return interact(player); }
			std::string getName() const override;
			void encode(Buffer &) override;
			void decode(Buffer &) override;

		private:
			ItemEntity(): Entity(ID()) {}
			ItemEntity(const Game &, const nlohmann::json &);
			ItemEntity(ItemStack);
			float xOffset = 0.f;
			float yOffset = 0.f;
			bool needsTexture = true;
			Atomic<float> secondsLeft = 5 * 60;

			ItemStack stack;

			void setTexture(const Game &);
			bool interact(const std::shared_ptr<Player> &);

		friend class Entity;
	};

	void to_json(nlohmann::json &, const ItemEntity &);
}
