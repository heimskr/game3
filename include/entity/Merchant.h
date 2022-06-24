#pragma once

#include "entity/Entity.h"

namespace Game3 {
	class Merchant: public virtual Entity {
		public:
			static std::shared_ptr<Merchant> create(EntityID);
			static std::shared_ptr<Merchant> fromJSON(const nlohmann::json &);

			MoneyCount money = 0;
			double greed = .1;

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(const nlohmann::json &) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &) override;

			friend class Entity;

		protected:
			Merchant(EntityID);
			void interact(const Position &);

		private:
			float accumulatedTime = 0.f;
			Direction lastDirection = Direction::Down;
	};

	void to_json(nlohmann::json &, const Merchant &);
}
