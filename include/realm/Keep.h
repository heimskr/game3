#pragma once

#include "realm/Realm.h"

namespace Game3 {
	class Keep: public Realm {
		public:
			Position parentOrigin;
			Index parentWidth;
			Index parentHeight;

			Keep(const Keep &) = delete;
			Keep(Keep &&) = default;
			~Keep() override = default;

			Keep & operator=(const Keep &) = delete;
			Keep & operator=(Keep &&) = default;

			friend class Realm;

		protected:
			Keep() = default;
			Keep(RealmID, const Position &parent_origin, Index parent_width, Index parent_height, const std::shared_ptr<Tilemap> &tilemap1_, const std::shared_ptr<Tilemap> &tilemap2_,
			     const std::shared_ptr<Tilemap> &tilemap3_);
			Keep(RealmID, const Position &parent_origin, Index parent_width, Index parent_height, const std::shared_ptr<Tilemap> &tilemap1_);

			void absorbJSON(const nlohmann::json &) override;
			void toJSON(nlohmann::json &) const override;
	};
}
