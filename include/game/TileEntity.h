#pragma once

#include <memory>

#include <nlohmann/json.hpp>

#include "Types.h"

namespace Game3 {
	class TileEntity {
		public:
			TileID tileID = 0;
			int row = -1;
			int column = -1;
			bool solid = false;

			TileEntity() = default;
			TileEntity(TileID tile_id, int row_, int column_, bool solid_): tileID(tile_id), row(row_), column(column_), solid(solid_) {}
			virtual ~TileEntity() = default;

			static std::shared_ptr<TileEntity> fromJSON(const nlohmann::json &);

			/** Returns the TileEntity ID, not the tile ID, which corresponds to a tile in the tileset. */
			virtual int getID() const = 0;

		protected:
			virtual void toJSON(nlohmann::json &) const;

			friend void to_json(nlohmann::json &, const TileEntity &);
	};

	void to_json(nlohmann::json &, const TileEntity &);
	void from_json(const nlohmann::json &, TileEntity &);
}
