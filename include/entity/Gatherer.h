#pragma once

#include "entity/Entity.h"

namespace Game3 {
	class Building;

	/** Lives in a town and gathers resources during the day. */
	class Gatherer: public Entity {
		public:
			constexpr static Index RADIUS = 50;
			constexpr static float HARVESTING_TIME = 5.f;
			constexpr static float SELLING_TIME = 5.f;
			constexpr static float WORK_START_HOUR = 8.f;
			constexpr static float WORK_END_HOUR = 18.f;
			constexpr static float RETRY_TIME = 30.f;

			Phase phase = 0;
			RealmID overworldRealm;
			RealmID houseRealm;
			Position housePosition;
			std::shared_ptr<Building> keep;
			Index chosenResource = -1;
			Position destination = {-1, -1};
			float harvestingTime;
			bool stuck = false;
			float stuckTime = 0.f;

			static std::shared_ptr<Gatherer> create(EntityID, RealmID overworld_realm, RealmID house_realm, const Position &house_position, const std::shared_ptr<Building> &keep_);
			static std::shared_ptr<Gatherer> fromJSON(const nlohmann::json &);

			nlohmann::json toJSON() const override;
			void absorbJSON(const nlohmann::json &) override;
			void initAfterRealm() override;
			bool onInteractNextTo(const std::shared_ptr<Player> &) override;
			void tick(Game &, float delta) override;

			friend class Entity;

		protected:
			Gatherer(EntityID id_);
			Gatherer(EntityID, RealmID overworld_realm, RealmID house_realm, const Position &house_position, const std::shared_ptr<Building> &keep_);

			void interact(const Position &);

		private:
			Position keepPosition;
			float sellTime = 0.f;
			Direction lastDirection = Direction::Down;

			void wakeUp();
			void goToResource();
			void startHarvesting();
			void harvest(float delta);
			void goToKeep();
			void goToStockpile();
			void sellInventory();
			void leaveKeep();
			void goToHouse();
			void goToBed();
			void setMoney(MoneyCount);
	};

	void to_json(nlohmann::json &, const Gatherer &);
}
