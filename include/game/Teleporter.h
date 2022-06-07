#pragma once

#include "game/TileEntity.h"

namespace Game3 {
	class Teleporter: public TileEntity {
		public:
			RealmID targetRealm;
			Position targetPosition;

			Teleporter(const Teleporter &) = delete;
			Teleporter(Teleporter &&) = default;
			~Teleporter() override = default;

			Teleporter & operator=(const Teleporter &) = delete;
			Teleporter & operator=(Teleporter &&) = default;

			TileEntityID getID() const override { return TileEntity::TELEPORTER; }

			void toJSON(nlohmann::json &) const override;
			void onOverlap(const std::shared_ptr<Entity> &) override;
			void absorbJSON(const nlohmann::json &) override;

		protected:
			Teleporter() = default;
			Teleporter(TileID id_, const Position &position_, RealmID target_realm, const Position &target_position):
				TileEntity(id_, TileEntity::TELEPORTER, position_, false), targetRealm(target_realm), targetPosition(target_position) {}

			friend class TileEntity;
	};
}
