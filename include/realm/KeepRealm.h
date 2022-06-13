#pragma once

#include "realm/Realm.h"

namespace Game3 {
	class KeepRealm: public Realm {
		public:
			Position parentOrigin;
			Index parentWidth;
			Index parentHeight;

			KeepRealm(const KeepRealm &) = delete;
			KeepRealm(KeepRealm &&) = default;
			~KeepRealm() override = default;

			KeepRealm & operator=(const KeepRealm &) = delete;
			KeepRealm & operator=(KeepRealm &&) = default;

			friend class Realm;

		protected:
			KeepRealm() = default;
			KeepRealm(RealmID, const Position &parent_origin, Index parent_width, Index parent_height, const std::shared_ptr<Tilemap> &tilemap1_, const std::shared_ptr<Tilemap> &tilemap2_,
			          const std::shared_ptr<Tilemap> &tilemap3_);
			KeepRealm(RealmID, const Position &parent_origin, Index parent_width, Index parent_height, const std::shared_ptr<Tilemap> &tilemap1_);

			void absorbJSON(const nlohmann::json &) override;
			void toJSON(nlohmann::json &) const override;
	};
}
