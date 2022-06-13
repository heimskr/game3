#pragma once

#include "tileentity/Building.h"

namespace Game3 {
	class Town: public Building {
		public:
			Index width;
			Index height;

			Town(const Town &) = delete;
			Town(Town &&) = default;
			~Town() override = default;

			Town & operator=(const Town &) = delete;
			Town & operator=(Town &&) = default;

			TileEntityID getID() const override { return TileEntity::TOWN; }

			void toJSON(nlohmann::json &) const override;
			// void onInteractNextTo(const std::shared_ptr<Player> &) override;
			void absorbJSON(const nlohmann::json &) override;

		protected:
			Town() = default;
			Town(TileID id_, const Position &position_, RealmID inner_realm_id, Index entrance_, Index width_, Index height_);

			friend class TileEntity;
	};
}
