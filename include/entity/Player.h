#pragma once

#include "entity/Entity.h"

namespace Game3 {
	class Player: public Entity {
		public:
			static std::shared_ptr<Player> fromJSON(const nlohmann::json &);
			bool movingUp = false;
			bool movingRight = false;
			bool movingDown = false;
			bool movingLeft = false;

			nlohmann::json toJSON() const override;
			bool isPlayer() const override { return true; }
			void tick(float delta) override;
			void remove() override {}
			void interactOn();
			void interactNextTo();
			using Entity::teleport;
			void teleport(const Position &, const std::shared_ptr<Realm> &) override;

		protected:
			using Entity::Entity;
			void interact(const Position &);
	};

	void to_json(nlohmann::json &, const Player &);
}
