#pragma once

#include "entity/Entity.h"

namespace Game3 {
	/** Lives in a town and gathers resources during the day. */
	class Gatherer: public Entity {
		public:
			constexpr static Index RADIUS = 50;
			constexpr static float HARVESTING_TIME = 1.f;

			Phase phase = 0;
			RealmID overworldRealm;
			RealmID houseRealm;
			Position housePosition;
			Index chosenResource = -1;
			Position resourceAdjacentPosition = {-1, -1};
			float harvestingTime;

			static std::shared_ptr<Gatherer> create(EntityID, RealmID overworld_realm, RealmID house_realm, const Position &house_position);
			static std::shared_ptr<Gatherer> fromJSON(const nlohmann::json &);

			nlohmann::json toJSON() const override;
			void absorbJSON(const nlohmann::json &) override;
			void tick(Game &, float delta) override;

			friend class Entity;

		protected:
			Gatherer(EntityID id_);
			Gatherer(EntityID, RealmID overworld_realm, RealmID house_realm, const Position &house_position);
			void interact(const Position &);

		private:
			float accumulatedTime = 0.f;
			Direction lastDirection = Direction::Down;
	};

	void to_json(nlohmann::json &, const Gatherer &);
}
