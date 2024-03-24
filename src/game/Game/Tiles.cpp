#include "game/Crop.h"
#include "game/Game.h"
#include "graphics/Tileset.h"
#include "tile/CaveTile.h"
#include "tile/CropTile.h"
#include "tile/DirtTile.h"
#include "tile/FarmlandTile.h"
#include "tile/ForestFloorTile.h"
#include "tile/GrassTile.h"
#include "tile/Tile.h"
#include "tile/TorchTile.h"
#include "tile/TreeTile.h"

namespace Game3 {
	std::shared_ptr<Tile> Game::getTile(const Identifier &identifier) {
		auto &reg = registry<TileRegistry>();
		if (auto found = reg.maybe(identifier))
			return found;
		static auto default_tile = std::make_shared<Tile>("base:tile/?");
		return default_tile;
	}

	void Game::addTiles() {
		GamePtr self = shared_from_this();
		auto &reg = registry<TileRegistry>();

		reg.add<DirtTile>();
		reg.add<FarmlandTile>();
		reg.add<ForestFloorTile>();
		reg.add<TorchTile>();
		reg.addMineable("base:tile/stone", ItemStack::create(self, "base:item/stone"), true);

		reg.add("base:tile/cave_coal",     std::make_shared<CaveTile>("base:tile/cave_coal",     ItemStack::create(self, "base:item/coal"),        "base:tile/cave_dirt"));
		reg.add("base:tile/cave_copper",   std::make_shared<CaveTile>("base:tile/cave_copper",   ItemStack::create(self, "base:item/copper_ore"),  "base:tile/cave_dirt"));
		reg.add("base:tile/cave_diamond",  std::make_shared<CaveTile>("base:tile/cave_diamond",  ItemStack::create(self, "base:item/diamond_ore"), "base:tile/cave_dirt"));
		reg.add("base:tile/cave_gold",     std::make_shared<CaveTile>("base:tile/cave_gold",     ItemStack::create(self, "base:item/gold_ore"),    "base:tile/cave_dirt"));
		reg.add("base:tile/cave_iron",     std::make_shared<CaveTile>("base:tile/cave_iron",     ItemStack::create(self, "base:item/iron_ore"),    "base:tile/cave_dirt"));
		reg.add("base:tile/cave_wall",     std::make_shared<CaveTile>("base:tile/cave_wall",     ItemStack::create(self, "base:item/stone"),       "base:tile/cave_dirt"));

		reg.add("base:tile/grimstone",     std::make_shared<CaveTile>("base:tile/grimstone",     ItemStack::create(self, "base:item/grimstone"),   "base:tile/grimdirt"));
		reg.add("base:tile/grim_diamond",  std::make_shared<CaveTile>("base:tile/grim_diamond",  ItemStack::create(self, "base:item/diamond_ore"), "base:tile/grimdirt"));
		reg.add("base:tile/grim_uranium",  std::make_shared<CaveTile>("base:tile/grim_uranium",  ItemStack::create(self, "base:item/uranium_ore"), "base:tile/grimdirt"));
		reg.add("base:tile/grim_fireopal", std::make_shared<CaveTile>("base:tile/grim_fireopal", ItemStack::create(self, "base:item/fire_opal"),   "base:tile/grimdirt"));

		static std::vector<std::string> rares {
			"apatite",
			"asbestos",
			"bauxite",
			"bornite",
			"cassiterite",
			"chalcopyrite",
			"chromite",
			"chrysoberyl",
			"garnet",
			"ilmenite",
			"magnetite",
			"malachite",
			"mica",
			"olivine",
			"pyrite",
			"pyrochlore",
			"pyrope",
			"sodalite",
			"stibnite",
			"sulfur",
			"tetrahedrite",
			"vanadium_magnetite",
		};

		for (const std::string &rare: rares) {
			Identifier tile("base:tile/cave_" + rare);
			Identifier item("base:item/" + rare + "_ore");
			reg.add(tile, std::make_shared<CaveTile>(tile, ItemStack::create(self, item), "base:tile/cave_dirt"));
		}

		const auto monomap = registry<TilesetRegistry>().at("base:tileset/monomap");
		auto grass = std::make_shared<GrassTile>();
		for (const auto &tilename: monomap->getTilesByCategory("base:category/flower_spawners"))
			reg.add(tilename, grass);

		for (const auto &[crop_name, crop]: registry<CropRegistry>()) {
			if (crop->customType.empty()) {
				auto tile = std::make_shared<CropTile>(crop);
				for (const auto &stage: crop->stages)
					reg.add(stage, tile);
			} else if (crop->customType == "base:tile/tree") {
				// TODO: We need the factory thing.
				auto tile = std::make_shared<TreeTile>(crop);
				for (const auto &stage: crop->stages)
					reg.add(stage, tile);
			}
		}
	}
}
