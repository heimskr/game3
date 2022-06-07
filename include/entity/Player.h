#pragma once

#include "entity/Entity.h"

namespace Game3 {
	class Player: public Entity {
		public:
			static std::shared_ptr<Player> fromJSON(const nlohmann::json &);

			nlohmann::json toJSON() const override;
			bool isPlayer() const override { return true; }
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
	void from_json(const nlohmann::json &, Player &);
}
