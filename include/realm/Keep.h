#pragma once

#include "realm/Realm.h"

namespace Game3 {
	class Inventory;

	class Keep: public Realm {
		public:
			Position parentOrigin;
			Index parentWidth;
			Index parentHeight;
			MoneyCount money = 1'000'000;
			double greed = .05;
			std::shared_ptr<Inventory> stockpileInventory;

			Keep(const Keep &) = delete;
			Keep(Keep &&) = default;
			~Keep() override = default;

			Keep & operator=(const Keep &) = delete;
			Keep & operator=(Keep &&) = default;

			friend class Realm;

		protected:
			Keep() = default;

			Keep(RealmID, const Position &parent_origin, Index parent_width, Index parent_height, TilemapPtr tilemap1_, TilemapPtr tilemap2_, TilemapPtr tilemap3_, BiomeMapPtr, int seed_);

			Keep(RealmID, const Position &parent_origin, Index parent_width, Index parent_height, TilemapPtr tilemap1_, BiomeMapPtr, int seed_);

			void absorbJSON(const nlohmann::json &) override;
			void toJSON(nlohmann::json &) const override;
	};
}
