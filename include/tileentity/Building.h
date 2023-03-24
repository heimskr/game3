#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class Building: public TileEntity {
		public:
			RealmID innerRealmID = 0;
			Index entrance = 0;

			Building(const Building &) = delete;
			Building(Building &&) = default;
			~Building() override = default;

			Building & operator=(const Building &) = delete;
			Building & operator=(Building &&) = default;

			TileEntityID getID() const override { return TileEntity::BUILDING; }

			void init() override {}
			void toJSON(nlohmann::json &) const override;
			bool onInteractOn(const std::shared_ptr<Player> &) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &) override;
			void absorbJSON(const nlohmann::json &) override;
			void teleport(const std::shared_ptr<Entity> &);
			void render(SpriteRenderer &) override;
			std::shared_ptr<Realm> getInnerRealm() const;

		protected:
			Building() = default;
			Building(TileID id_, const Position &position_, RealmID inner_realm_id, Index entrance_):
				TileEntity(id_, TileEntity::BUILDING, position_, true), innerRealmID(inner_realm_id), entrance(entrance_) {}

			friend class TileEntity;
	};
}
