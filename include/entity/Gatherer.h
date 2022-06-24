#pragma once

#include "entity/Worker.h"

namespace Game3 {
	class Building;

	/** Lives in a town and gathers resources during the day. */
	class Gatherer: public Worker {
		public:
			constexpr static Index RADIUS = 50;
			constexpr static float HARVESTING_TIME = 5.f;
			constexpr static float SELLING_TIME = 5.f;

			Index chosenResource = -1;
			float harvestingTime;

			static std::shared_ptr<Gatherer> create(EntityID, RealmID overworld_realm, RealmID house_realm, const Position &house_position, const std::shared_ptr<Building> &keep_);
			static std::shared_ptr<Gatherer> fromJSON(const nlohmann::json &);

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(const nlohmann::json &) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &) override;
			void tick(Game &, float delta) override;

			friend class Entity;

		protected:
			Gatherer(EntityID);
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
