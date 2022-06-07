#pragma once

#include "game/TileEntity.h"

namespace Game3 {
	class Building: public TileEntity {
		public:
			static constexpr int ID = 1;

			RealmID innerRealmID = 0;
			Index entrance = 0;

			Building(const Building &) = delete;
			Building(Building &&) = default;
			~Building() override = default;

			Building & operator=(const Building &) = delete;
			Building & operator=(Building &&) = default;

			TileEntityID getID() const override { return TileEntity::BUILDING; }

			void toJSON(nlohmann::json &) const override;
			void onInteractNextTo(const std::shared_ptr<Player> &) override;
			void absorbJSON(const nlohmann::json &) override;

		protected:
			Building() = default;
			Building(TileID id_, const Position &position_, RealmID inner_realm_id, Index entrance_):
				TileEntity(id_, TileEntity::BUILDING, position_, true), innerRealmID(inner_realm_id), entrance(entrance_) {}

			friend class TileEntity;
	};
}
