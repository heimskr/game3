#pragma once

#include "mixin/HasSoundSet.h"
#include "tileentity/TileEntity.h"

namespace Game3 {
	class Teleporter: public TileEntity, public HasSoundSet {
		public:
			static Identifier ID() { return {"base", "te/teleporter"}; }
			RealmID targetRealm;
			Position targetPosition;

			std::string getName() const override { return "Teleporter"; }

			GamePtr getGame() const override;
			void toJSON(boost::json::value &) const override;
			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;
			void onOverlap(const std::shared_ptr<Entity> &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		protected:
			Teleporter() = default;
			Teleporter(Identifier tilename, Position position, RealmID targetRealm, Position targetPosition, Identifier soundSetID);

		friend class TileEntity;
	};
}
