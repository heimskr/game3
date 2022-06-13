#pragma once

#include "entity/Entity.h"

namespace Game3 {
	/** Lives in a town and gathers resources during the day. */
	class Gatherer: public virtual Entity {
		public:
			static std::shared_ptr<Gatherer> create(EntityID);
			static std::shared_ptr<Gatherer> fromJSON(const nlohmann::json &);

			nlohmann::json toJSON() const override;
			void absorbJSON(const nlohmann::json &) override;
			void onInteractNextTo(const std::shared_ptr<Player> &) override;

		protected:
			using Entity::Entity;
			void interact(const Position &);

		private:
			float accumulatedTime = 0.f;
			Direction lastDirection = Direction::Down;
	};

	void to_json(nlohmann::json &, const Gatherer &);
}
