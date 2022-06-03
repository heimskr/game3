#pragma once

#include <memory>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include "game/TileEntity.h"
#include "Tilemap.h"
#include "Types.h"

namespace Game3 {
	struct Realm {
		int id;
		std::shared_ptr<Tilemap> tilemap;
		std::unordered_map<Index, std::shared_ptr<TileEntity>> tileEntities;

		Realm(int id_, const std::shared_ptr<Tilemap> &tilemap_): id(id_), tilemap(tilemap_) {}
	};

	void to_json(nlohmann::json &, const Realm &);
}
