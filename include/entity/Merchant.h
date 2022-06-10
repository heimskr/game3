#pragma once

#include "entity/Entity.h"

namespace Game3 {
	class Merchant: public virtual Entity {
		public:
			static std::shared_ptr<Merchant> create(EntityID);
			static std::shared_ptr<Merchant> fromJSON(const nlohmann::json &);

			double priceMultiplier = 1.;

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

	void to_json(nlohmann::json &, const Merchant &);
}
