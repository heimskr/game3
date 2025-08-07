#pragma once

#include "entity/Entity.h"
#include "math/Vector.h"

namespace Game3 {
	class Ship: public Entity {
		public:
			static Identifier ID() { return {"base", "entity/ship"}; }

			static std::shared_ptr<Ship> create(const std::shared_ptr<Game> &) {
				return Entity::create<Ship>();
			}

			std::string getName() const override { return "Ship"; }

			void onSpawn() override;
			void onDestroy() override;
			void updateRiderOffset(const EntityPtr &rider) override;
			RideType getRideType() const override { return RideType::Hidden; }
			bool moveFromRider(const EntityPtr &, Direction, MovementContext) override;
			float getMovementSpeed() const override;
			bool canMoveTo(const Place &) const override;
			bool onInteractOn(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			bool interactable(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &used_item, Hand) override;
			void render(const RendererContext &) override;
			void encode(Buffer &) override;
			void decode(Buffer &) override;

			Vector2i getDimensions() const override { return {2, 2}; }

		protected:
			RealmID internalRealmID = 0;
			Position realmOrigin{CHUNK_SIZE / 2, CHUNK_SIZE / 2};

			Ship();

			void teleportToRealm(const std::shared_ptr<Player> &);

		friend class Entity;
	};
}
