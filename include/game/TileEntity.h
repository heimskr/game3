#pragma once

#include <nlohmann/json.hpp>

#include "Types.h"

namespace Game3 {
	class TileEntity {
		public:
			TileID id;
			int row;
			int column;
			bool solid;

			TileEntity(TileID id_, int row_, int column_, bool solid_): id(id_), row(row_), column(column_), solid(solid_) {}
			virtual ~TileEntity() = default;

		protected:
			virtual void toJSON(nlohmann::json &) const;

			friend void to_json(nlohmann::json &, const TileEntity &);
	};

	void to_json(nlohmann::json &, const TileEntity &);
	void from_json(const nlohmann::json &, TileEntity &);
}
