#pragma once

#include "realm/Realm.h"

namespace Game3 {
	class Inventory;

	class Cave: public Realm {
		public:
			RealmID parentRealm;
			size_t entranceCount = 1;

			Cave(const Cave &) = delete;
			Cave(Cave &&) = default;
			~Cave() override;

			Cave & operator=(const Cave &) = delete;
			Cave & operator=(Cave &&) = default;

			friend class Realm;

		protected:
			Cave() = default;

			Cave(RealmID, RealmID parent_realm, const std::shared_ptr<Tilemap> &tilemap1_, const std::shared_ptr<Tilemap> &tilemap2_,
			     const std::shared_ptr<Tilemap> &tilemap3_, int seed_);

			Cave(RealmID, RealmID parent_realm, const std::shared_ptr<Tilemap> &tilemap1_, int seed_);

			void absorbJSON(const nlohmann::json &) override;
			void toJSON(nlohmann::json &) const override;
	};
}
