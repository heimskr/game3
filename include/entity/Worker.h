#pragma once

#include "entity/Entity.h"

namespace Game3 {
	class Building;

	/** Lives in a town. */
	class Worker: public virtual Entity {
		public:
			constexpr static float WORK_START_HOUR = 8.f;
			constexpr static float WORK_END_HOUR = 18.f;
			constexpr static float RETRY_TIME = 30.f;

			Phase phase = 0;
			RealmID overworldRealm;
			RealmID houseRealm;
			Position housePosition;
			std::shared_ptr<Building> keep;
			Position destination = {-1, -1};
			bool stuck = false;
			float stuckTime = 0.f;

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(const nlohmann::json &) override;
			void initAfterLoad(Game &) override;

			friend class Entity;

		protected:
			Position keepPosition;
			Direction lastDirection = Direction::Down;

			Worker(EntityID, EntityType);
			Worker(EntityID, EntityType, RealmID overworld_realm, RealmID house_realm, const Position &house_position, const std::shared_ptr<Building> &keep_);

			void interact(const Position &);
			bool stillStuck(float delta);
			void goToKeep(Phase new_phase);
			void goToStockpile(Phase new_phase);
			void leaveKeep(Phase new_phase);
			void goToHouse(Phase new_phase);
			void goToBed(Phase new_phase);

			inline void setMoney(MoneyCount new_money) { money = new_money; }
	};
}
