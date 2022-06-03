#pragma once

#include "game/TileEntity.h"

namespace Game3 {
	struct Building: TileEntity {
		static constexpr int ID = 1;

		int innerRealmID = -1;

		Building() = default;
		Building(TileID id_, int row_, int column_, int inner_realm_id): TileEntity(id_, row_, column_, false), innerRealmID(inner_realm_id) {}

		~Building() override = default;

		int getID() const override { return ID; }

		void toJSON(nlohmann::json &) const override;
	};

	void from_json(const nlohmann::json &, Building &);
}
