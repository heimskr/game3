#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class Building: public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/building"}; }
			RealmID innerRealmID = 0;
			Position entrance;

			Building(const Building &) = delete;
			Building(Building &&) = default;
			~Building() override = default;

			Building & operator=(const Building &) = delete;
			Building & operator=(Building &&) = default;

			void toJSON(nlohmann::json &) const override;
			bool onInteractOn(const std::shared_ptr<Player> &, Modifiers, ItemStack *, Hand) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, ItemStack *, Hand) override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			void teleport(const std::shared_ptr<Entity> &);
			std::shared_ptr<Realm> getInnerRealm() const;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		protected:
			Building() = default;
			Building(Identifier, Position, RealmID inner_realm_id, Position entrance_);

			friend class TileEntity;
	};
}
