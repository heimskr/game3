#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class Teleporter: public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/teleporter"}; }
			RealmID targetRealm;
			Position targetPosition;

			Teleporter(const Teleporter &) = delete;
			Teleporter(Teleporter &&) = default;
			~Teleporter() override = default;

			Teleporter & operator=(const Teleporter &) = delete;
			Teleporter & operator=(Teleporter &&) = default;

			void init(Game &) override {}
			void toJSON(nlohmann::json &) const override;
			void onOverlap(const std::shared_ptr<Entity> &) override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			void render(SpriteRenderer &) override;

		protected:
			Teleporter() = default;
			Teleporter(Identifier tilename, Position position_, RealmID target_realm, Position target_position);

			friend class TileEntity;
	};
}
