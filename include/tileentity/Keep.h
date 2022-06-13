#pragma once

#include "tileentity/Building.h"

namespace Game3 {
	class Keep: public Building {
		public:
			Position origin;
			Index width;
			Index height;

			Keep(const Keep &) = delete;
			Keep(Keep &&) = default;
			~Keep() override = default;

			Keep & operator=(const Keep &) = delete;
			Keep & operator=(Keep &&) = default;

			TileEntityID getID() const override { return TileEntity::KEEP; }

			void toJSON(nlohmann::json &) const override;
			// void onInteractNextTo(const std::shared_ptr<Player> &) override;
			void absorbJSON(const nlohmann::json &) override;

		protected:
			Keep() = default;
			Keep(TileID id_, const Position &position_, RealmID inner_realm_id, Index entrance_, const Position &origin_, Index width_, Index height_);

			friend class TileEntity;
	};
}
