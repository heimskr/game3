#pragma once

#include "graphics/Texture.h"
#include "entity/Entity.h"
#include "item/Item.h"
#include "threading/Atomic.h"

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class Game;

	class ItemEntity: public Entity {
		public:
			static Identifier ID() { return {"base", "entity/item"}; }
			inline ItemStackPtr getStack() const { return stack; }
			void setStack(ItemStackPtr);

			static std::shared_ptr<ItemEntity> create(const GamePtr &);
			static std::shared_ptr<ItemEntity> create(const GamePtr &, ItemStackPtr);
			static std::shared_ptr<ItemEntity> fromJSON(const GamePtr &, const nlohmann::json &);

			void toJSON(nlohmann::json &) const override;
			void init(const GamePtr &) override;
			void tick(const TickArgs &) override;
			void render(const RendererContext &) override;
			bool onInteractOn    (const PlayerPtr &player, Modifiers, const ItemStackPtr &, Hand) override { return interact(player); }
			bool onInteractNextTo(const PlayerPtr &player, Modifiers, const ItemStackPtr &, Hand) override { return interact(player); }
			std::string getName() const override;
			void encode(Buffer &) override;
			void decode(Buffer &) override;
			int getZIndex() const override { return -1; }

		private:
			ItemEntity(): Entity(ID()) {}
			ItemEntity(const GamePtr &);
			ItemEntity(ItemStackPtr);
			float offsetX = 0.f;
			float offsetY = 0.f;
			float sizeX = 16.f;
			float sizeY = 16.f;
			bool needsTexture = true;
			Atomic<int> secondsLeft = 5 * 60;
			bool firstTick = true;

			ItemStackPtr stack;

			void setTexture(const GamePtr &);
			bool interact(const PlayerPtr &);

		friend class Entity;
	};

	void to_json(nlohmann::json &, const ItemEntity &);
}
