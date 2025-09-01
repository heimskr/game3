#pragma once

#include "data/SoundSet.h"
#include "mixin/HasSoundSet.h"
#include "tileentity/TileEntity.h"

namespace Game3 {
	class Building: public TileEntity, public HasSoundSet {
		public:
			static Identifier ID() { return {"base", "te/building"}; }
			RealmID innerRealmID = 0;
			Position entrance;

			std::string getName() const override { return "Building"; }

			GamePtr getGame() const override;
			void toJSON(boost::json::value &) const override;
			bool onInteractOn(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;
			void teleport(const std::shared_ptr<Entity> &);
			std::shared_ptr<Realm> getInnerRealm() const;

			void encode(Game &, Buffer &) override;
			void decode(Game &, BasicBuffer &) override;

		protected:
			Building() = default;
			Building(Identifier, Position, RealmID innerRealmID, Position entrance, Identifier soundSetID = {});

		friend class TileEntity;
	};
}
