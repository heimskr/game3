#pragma once

#include "entity/Entity.h"

namespace Game3 {
	class Ship: public Entity {
		public:
			static Identifier ID() { return {"base", "entity/ship"}; }

			static std::shared_ptr<Ship> create(Game &) {
				return Entity::create<Ship>();
			}

			std::string getName() const override { return "Ship"; }

			void updateRiderOffset(const std::shared_ptr<Entity> &rider) override;
			RideType getRideType() const override { return RideType::Hidden; }
			bool moveFromRider(Direction, MovementContext) override;
			bool onInteractOn(const std::shared_ptr<Player> &, Modifiers, ItemStack *, Hand) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, ItemStack *, Hand) override;
			void render(const RendererContext &) override;
			void encode(Buffer &) override;
			void decode(Buffer &) override;

			Vector2i getDimensions() const override { return {2, 2}; }

		protected:
			Ship();

		friend class Entity;
	};
}
