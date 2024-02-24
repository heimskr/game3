#pragma once

#include "entity/LivingEntity.h"

namespace Game3 {
	class Building;

	/** Lives in a town. */
	class Worker: public LivingEntity {
		public:
			static Identifier ID() { return {"base", "entity/worker"}; }
			constexpr static double WORK_START_HOUR = 8.;
			constexpr static double WORK_END_HOUR = 18.;
			constexpr static double RETRY_TIME = 30.;
			constexpr static HitPoints MAX_HEALTH = 40;

			Phase phase = 0;
			RealmID overworldRealm = -1;
			RealmID houseRealm = -1;
			Position housePosition;
			std::shared_ptr<Building> keep;
			Position destination = {-1, -1};
			bool stuck = false;
			double stuckTime = 0.;

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			void initAfterLoad(Game &) override;
			void encode(Buffer &) override;
			void decode(Buffer &) override;

			friend class Entity;

		protected:
			Position keepPosition;

			Worker(EntityType);
			Worker(EntityType, RealmID overworld_realm, RealmID house_realm, Position house_position, std::shared_ptr<Building> keep_);

			HitPoints getMaxHealth() const override { return MAX_HEALTH; }
			bool stillStuck(double delta);
			void goToKeep(Phase new_phase);
			void goToStockpile(Phase new_phase);
			void leaveKeep(Phase new_phase);
			void goToHouse(Phase new_phase);
			void goToBed(Phase new_phase);

			void setMoney(MoneyCount);
			void setPhase(Phase);
	};
}
