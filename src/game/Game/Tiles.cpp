#include "game/Crop.h"
#include "game/Game.h"
#include "graphics/Tileset.h"
#include "tile/BedTile.h"
#include "tile/CaveTile.h"
#include "tile/CropTile.h"
#include "tile/DirtTile.h"
#include "tile/FarmlandTile.h"
#include "tile/FenceGateTile.h"
#include "tile/FiniteShovelableTile.h"
#include "tile/FoodTile.h"
#include "tile/ForestFloorTile.h"
#include "tile/GrassTile.h"
#include "tile/SnowTile.h"
#include "tile/Tile.h"
#include "tile/TorchTile.h"
#include "tile/TreeTile.h"
#include "tile/VoidTile.h"

namespace Game3 {
	std::shared_ptr<Tile> Game::getTile(const Identifier &identifier) {
		TileRegistry &reg = *tileRegistry;
		if (auto found = reg.maybe(identifier)) {
			return found;
		}
		static auto default_tile = std::make_shared<Tile>("base:tile/?");
		return default_tile;
	}

	void Game::addTiles() {
		GamePtr self = shared_from_this();
		TileRegistry &reg = *tileRegistry;

		reg.add<DirtTile>();
		reg.add<FarmlandTile>();
		reg.add<ForestFloorTile>();
		reg.add<SnowTile>();
		reg.add<TorchTile>();
		reg.add<VoidTile>();
		reg.add<BedTile>("base:tile/bed1");
		reg.add<BedTile>("base:tile/bed2");
		reg.add<BedTile>("base:tile/bed3");
		reg.addMineable("base:tile/stone", ItemStack::create(self, "base:item/stone"), true);

		for (Identifier id: {"base:tile/gate_horizontal", "base:tile/gate_horizontal_n", "base:tile/gate_horizontal_s", "base:tile/gate_vertical", "base:tile/gate_vertical_e", "base:tile/gate_vertical_w"}) {
			reg.add<FenceGateTile>(std::move(id));
		}

		reg.add("base:tile/cave_coal",     std::make_shared<CaveTile>("base:tile/cave_coal",     ItemStack::create(self, "base:item/coal", 4),     Layer::Soil, "base:tile/cave_dirt"));
		reg.add("base:tile/cave_copper",   std::make_shared<CaveTile>("base:tile/cave_copper",   ItemStack::create(self, "base:item/copper_ore"),  Layer::Soil, "base:tile/cave_dirt"));
		reg.add("base:tile/cave_diamond",  std::make_shared<CaveTile>("base:tile/cave_diamond",  ItemStack::create(self, "base:item/diamond_ore"), Layer::Soil, "base:tile/cave_dirt"));
		reg.add("base:tile/cave_gold",     std::make_shared<CaveTile>("base:tile/cave_gold",     ItemStack::create(self, "base:item/gold_ore"),    Layer::Soil, "base:tile/cave_dirt"));
		reg.add("base:tile/cave_iron",     std::make_shared<CaveTile>("base:tile/cave_iron",     ItemStack::create(self, "base:item/iron_ore"),    Layer::Soil, "base:tile/cave_dirt"));
		reg.add("base:tile/cave_wall",     std::make_shared<CaveTile>("base:tile/cave_wall",     ItemStack::create(self, "base:item/stone"),       Layer::Soil, "base:tile/cave_dirt"));

		reg.add("base:tile/grimstone",     std::make_shared<CaveTile>("base:tile/grimstone",     ItemStack::create(self, "base:item/grimstone"),   Layer::Soil, "base:tile/grimdirt"));
		reg.add("base:tile/grim_diamond",  std::make_shared<CaveTile>("base:tile/grim_diamond",  ItemStack::create(self, "base:item/diamond_ore"), Layer::Soil, "base:tile/grimdirt"));
		reg.add("base:tile/grim_uranium",  std::make_shared<CaveTile>("base:tile/grim_uranium",  ItemStack::create(self, "base:item/uranium_ore"), Layer::Soil, "base:tile/grimdirt"));
		reg.add("base:tile/grim_fireopal", std::make_shared<CaveTile>("base:tile/grim_fireopal", ItemStack::create(self, "base:item/fire_opal"),   Layer::Soil, "base:tile/grimdirt"));

		reg.add("base:tile/cake", std::make_shared<FoodTile>("base:tile/cake", "base:item/cake"));

		auto finite = [&](const char *tile_name, const char *item_name) {
			Identifier tile_id{"base", std::string("tile/") + tile_name};
			Identifier item_id{"base", std::string("item/") + item_name};
			auto tile = std::make_shared<FiniteShovelableTile>(tile_id, std::move(item_id));
			reg.add(std::move(tile_id), std::move(tile));
		};

		auto infinite = [&](const char *tile_name, const char *item_name) {
			Identifier tile_id{"base", std::string("tile/") + tile_name};
			Identifier item_id{"base", std::string("item/") + item_name};
			auto tile = std::make_shared<InfiniteShovelableTile>(tile_id, std::move(item_id));
			reg.add(std::move(tile_id), std::move(tile));
		};

		finite("ash", "ash");
		infinite("volcanic_sand", "sulfur");
		infinite("sand", "sand");

		static const std::array rares{
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

		for (std::string rare: rares) {
			Identifier tile("base:tile/cave_" + rare);
			Identifier item("base:item/" + rare + "_ore");
			reg.add(tile, std::make_shared<CaveTile>(tile, ItemStack::create(self, item), Layer::Soil, "base:tile/cave_dirt"));
		}

		const auto monomap = registry<TilesetRegistry>().at("base:tileset/monomap");

		const auto &flower_spawners = monomap->getTilesByCategory("base:category/flower_spawners");
		for (const Identifier &tilename: flower_spawners) {
			reg.add(tilename, std::make_shared<GrassTile>(tilename));
		}

		for (const Identifier &tilename: monomap->getTilesByCategory("base:category/dirt")) {
			if (tilename != "base:tile/dirt" && !flower_spawners.contains(tilename)) {
				reg.add(tilename, std::make_shared<InfiniteShovelableTile>(tilename, "base:item/dirt"));
			}
		}

		for (const auto &[crop_name, crop]: registry<CropRegistry>()) {
			if (crop->customType.empty()) {
				auto tile = std::make_shared<CropTile>(crop);
				for (const auto &stage: crop->stages) {
					reg.add(stage, tile);
				}
			} else if (crop->customType == "base:tile/tree") {
				// TODO: We need the factory thing.
				auto tile = std::make_shared<TreeTile>(crop);
				for (const auto &stage: crop->stages) {
					reg.add(stage, tile);
				}
			}
		}

	}
}
