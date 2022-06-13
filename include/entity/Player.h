#pragma once

#include "entity/Entity.h"

namespace Game3 {
	class Player: public Entity {
		public:
			MoneyCount money;

			bool movingUp = false;
			bool movingRight = false;
			bool movingDown = false;
			bool movingLeft = false;
			bool ticked = false;

			static std::shared_ptr<Player> fromJSON(const nlohmann::json &);

			nlohmann::json toJSON() const override;
			void absorbJSON(const nlohmann::json &) override;
			bool isPlayer() const override { return true; }
			void tick(float delta) override;
			void remove() override {}
			void interactOn();
			void interactNextTo();
			using Entity::teleport;
			void teleport(const Position &, const std::shared_ptr<Realm> &) override;
			void addMoney(MoneyCount);

		protected:
			using Entity::Entity;
			void interact(const Position &);
	};

	void to_json(nlohmann::json &, const Player &);
}
