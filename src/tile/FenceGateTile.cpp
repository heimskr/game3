#include "Log.h"
#include "types/Position.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "graphics/Tileset.h"
#include "realm/Realm.h"
#include "registry/Registries.h"
#include "tile/FenceGateTile.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	bool FenceGateTile::interact(const Place &place, Layer layer, const ItemStackPtr &held_item, Hand hand) {
		auto id = place.getName(layer);
		if (!id)
			return false;

		const std::string &tile_name = id->get().name;
		const bool is_closed = place.realm->getTileset().isInCategory(*id, "base:category/fence_gates_closed");

		const Direction direction = place.player->getDirection();

		const char *new_id = nullptr;

		if (tile_name.starts_with("tile/gate_horizontal")) {
			if (is_closed)
				new_id = direction == Direction::Down? "base:tile/gate_horizontal_s" : "base:tile/gate_horizontal_n";
			else
				new_id = "base:tile/gate_horizontal";
		} else if (tile_name.starts_with("tile/gate_vertical")) {
			if (is_closed)
				new_id = direction == Direction::Right? "base:tile/gate_vertical_e" : "base:tile/gate_vertical_w";
			else
				new_id = "base:tile/gate_vertical";
		}

		assert(new_id);

		place.set(layer, Identifier(new_id));

		return true;
	}

	bool FenceGateTile::update(const Place &place, Layer layer) {
		auto id = place.getName(layer);
		if (!id)
			return false;

		Tileset &tileset = place.realm->getTileset();

		auto is_match = [&](Direction direction) {
			auto opt = (place + direction).getName(layer);
			if (!opt)
				return false;
			const Identifier &id = opt->get();
			return id == "base:tile/fence" || tileset.isInCategory(id, "base:category/fence_gates");
		};

		bool is_vertical = false;

		if (is_match(Direction::Left) || is_match(Direction::Right)) {
			// Leave is_vertical false
		} else if (is_match(Direction::Up) || is_match(Direction::Down)) {
			is_vertical = true;
		}

		const bool is_closed = tileset.isInCategory(*id, "base:category/fence_gates_closed");

		const std::string &tile_name = id->get().name;

		if (tile_name.starts_with("tile/gate_horizontal")) {
			if (is_vertical)
				place.set(layer, is_closed? "base:tile/gate_vertical" : "base:tile/gate_vertical_e");
		} else if (tile_name.starts_with("tile/gate_vertical")) {
			if (!is_vertical)
				place.set(layer, is_closed? "base:tile/gate_horizontal" : "base:tile/gate_horizontal_s");
		}

		return true;
	}
}
