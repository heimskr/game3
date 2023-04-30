#pragma once

#include "realm/Realm.h"

namespace Game3 {
	class Inventory;

	class Keep: public Realm {
		public:
			static Identifier ID() { return {"base", "realm/keep"}; }
			Position parentOrigin;
			Index parentWidth;
			Index parentHeight;
			MoneyCount money = 1'000'000;
			double greed = .05;
			std::shared_ptr<Inventory> stockpileInventory;

			Keep(const Keep &) = delete;
			Keep(Keep &&) = delete;
			~Keep() override = default;

			Keep & operator=(const Keep &) = delete;
			Keep & operator=(Keep &&) = delete;

			friend class Realm;

		protected:
			Keep(Game &);
			Keep(Game &, RealmID, const Position &parent_origin, Index parent_width, Index parent_height, int seed_);

			void absorbJSON(const nlohmann::json &) override;
			void toJSON(nlohmann::json &) const override;
	};
}
