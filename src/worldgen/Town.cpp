#include "game/Crop.h"
#include "game/Game.h"
#include "graphics/Tileset.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Arcade.h"
#include "worldgen/BlacksmithGen.h"
#include "worldgen/Keep.h"
#include "worldgen/House.h"
#include "worldgen/Tavern.h"
#include "worldgen/WorldGen.h"

namespace Game3::WorldGen {
	void generateTown(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, const Position &position, Index width, Index height, Index pad, int seed, const VillagePtr &village) {
		Index row = 0;
		Index column = 0;

		// auto pauser = realm->pauseUpdates();
		auto guard = realm->guardGeneration();

		const auto cleanup = [&](Index row, Index column) {
			if (auto tile_entity = realm->tileEntityAt({row, column}))
				realm->remove(tile_entity);
		};

		const auto set_terrain = [&](const Identifier &tilename, bool helper = false) {
			cleanup(row, column);
			realm->setTile(Layer::Terrain, {row, column}, tilename, helper);
		};

		const auto set_submerged = [&](const Position &position, const Identifier &tilename) {
			cleanup(position.row, position.column);
			realm->setTile(Layer::Submerged, position, tilename, false);
		};

		const auto set_objects = [&](const Identifier &tilename, bool helper = false) {
			cleanup(row, column);
			realm->setTile(Layer::Objects, {row, column}, tilename, helper);
		};

		const auto set_objects_pos = [&](const Position &position, const Identifier &tilename, bool helper = false) {
			cleanup(position.row, position.column);
			realm->setTile(Layer::Objects, position, tilename, helper);
		};

		Timer town_timer("TownLayout");
		for (row = position.row - pad; row < position.row + height + pad; ++row)
			for (column = position.column - pad; column < position.column + width + pad; ++column) {
				set_submerged({row, column}, "base:tile/empty");
				set_objects_pos({row, column}, "base:tile/empty");
			}

		for (row = position.row; row < position.row + height; ++row) {
			set_objects_pos({row, position.column}, "base:tile/tower");
			set_objects_pos({row, position.column + width - 1}, "base:tile/tower");
		}

		for (column = position.column; column < position.column + width; ++column) {
			set_objects_pos({position.row, column}, "base:tile/tower", true);
			set_objects_pos({position.row + height - 1, column}, "base:tile/tower", true);
		}

		set_objects_pos(position, "base:tile/tower", true);
		set_objects_pos({position.row + height - 1, position.column}, "base:tile/tower", true);
		set_objects_pos({position.row, position.column + width - 1}, "base:tile/tower", true);
		set_objects_pos({position.row + height - 1, position.column + width - 1}, "base:tile/tower", true);

		std::unordered_set<Position> buildable_set;

		for (row = position.row + 1; row < position.row + height - 1; ++row) {
			for (column = position.column + 1; column < position.column + width - 1; ++column) {
				buildable_set.insert({row, column});
				set_terrain("base:tile/town_dirt", true);
			}
		}

		row = position.row + height / 2 - 1;
		for (column = position.column - pad; column < position.column + width + pad; ++column) {
			buildable_set.erase({row, column});
			buildable_set.erase({position.row + height - 2,  column}); // Make sure no houses spawn on the bottom row of the town
			set_terrain("base:tile/road");
			++row;
			buildable_set.erase({row, column});
			set_terrain("base:tile/road");
			--row;
		}

		column = position.column;
		set_objects("base:tile/empty");
		--row;
		set_objects("base:tile/tower", true);
		row += 2;
		set_objects("base:tile/empty");
		++row;
		set_objects("base:tile/tower", true);
		--row;
		column += width - 1;
		set_objects("base:tile/empty");
		--row;
		set_objects("base:tile/empty");
		--row;
		set_objects("base:tile/tower", true);
		row += 3;
		set_objects("base:tile/tower", true);
		--row;
		column = position.column + width / 2 - 1;

		for (row = position.row - pad; row < position.row + height + pad; ++row) {
			buildable_set.erase({row, column});
			set_terrain("base:tile/road");
			++column;
			buildable_set.erase({row, column});
			set_terrain("base:tile/road");
			--column;
		}

		GamePtr game = realm->getGame();
		auto &crop_registry = game->registry<CropRegistry>();
		std::vector<std::shared_ptr<Crop>> crops;
		for (const auto &[key, crop]: crop_registry)
			if (crop->canSpawnInTown)
				crops.push_back(crop);

		for (row = position.row + 1; row < position.row + height / 2 - 1; ++row) {
			for (column = position.column + 1; column < position.column + width / 2 - 1; ++column) {
				buildable_set.erase({row, column});
				set_terrain("base:tile/farmland");
				set_submerged({row, column}, choose(crops, rng)->getLastStage());
			}
		}

		row = position.row;
		set_objects("base:tile/empty");
		--column;
		set_objects("base:tile/tower", true);
		column += 2;
		set_objects("base:tile/empty");
		++column;
		set_objects("base:tile/tower", true);
		column -= 2;
		row += height - 1;
		set_objects("base:tile/empty");
		--column;
		set_objects("base:tile/tower", true);
		column += 2;
		set_objects("base:tile/empty");
		++column;
		set_objects("base:tile/tower", true);
		--column;

		Position keep_position(position.row + height / 2 - 1, position.column + width / 2 - 1);
		const Position town_origin(position.row - pad, position.column - pad);
		const RealmID keep_realm_id = game->newRealmID();
		const Index keep_width = 15;
		const Index keep_height = 15;
		const Position keep_entrance(keep_height - 2, keep_width / 2);
		const Position keep_exit = keep_position + Position(2, 0);
		auto keep_biomemap = std::make_shared<BiomeMap>(keep_width, keep_height);
		auto keep_realm = Realm::create(game, keep_realm_id, "base:realm/keep"_id, "base:tileset/monomap"_id, -seed);
		keep_realm->outdoors = false;
		game->addRealm(keep_realm_id, keep_realm);
		WorldGen::generateKeep(keep_realm, rng, realm->id, keep_width, keep_height, keep_exit, village->getID());
		keep_realm->remakePathMap(ChunkRange({-1, -1}, {1, 1}));

		auto create_keep = [&](const Identifier &tilename) {
			realm->setTile(Layer::Objects, keep_position, tilename, false);
			TileEntity::spawn<Building>(realm, tilename, keep_position, keep_realm_id, keep_entrance);
		};

		create_keep("base:tile/keep_nw");
		++keep_position.column;
		create_keep("base:tile/keep_ne");
		++keep_position.row;
		create_keep("base:tile/keep_se");
		--keep_position.column;
		create_keep("base:tile/keep_sw");

		// Prevent houses from being placed on the corners around the keep
		buildable_set.erase({position.row + height / 2 - 2, position.column + width / 2 - 2});
		buildable_set.erase({position.row + height / 2 - 2, position.column + width / 2 + 1});
		buildable_set.erase({position.row + height / 2 + 1, position.column + width / 2 + 1});
		buildable_set.erase({position.row + height / 2 + 1, position.column + width / 2 - 2});

		town_timer.stop();
		std::vector<Position> buildable(buildable_set.cbegin(), buildable_set.cend());
		std::shuffle(buildable.begin(), buildable.end(), rng);
		Timer houses_timer("Houses");

		if (2 < buildable.size()) {
			buildable.erase(buildable.begin() + buildable.size() / 10, buildable.end());
			buildable_set = std::unordered_set<Position>(buildable.cbegin(), buildable.cend());

			Position building_position;

			auto gen_building = [&](const Identifier &tilename, Index realm_width, Index realm_height, RealmType realm_type, const BuildingGenerator &gen_fn, std::optional<Position> entrance = std::nullopt) {
				realm->setTile(Layer::Objects, building_position, tilename, false);
				const RealmID realm_id = game->newRealmID();
				auto building = TileEntity::spawn<Building>(realm, tilename, building_position, realm_id, entrance? *entrance : Position(realm_height - 2, realm_width - 3));
				assert(building);
				auto details = game->registry<RealmDetailsRegistry>()[realm_type];
				auto new_realm = Realm::create(game, realm_id, realm_type, details->tilesetName, -seed);
				new_realm->outdoors = false;
				gen_fn(new_realm, rng, realm, realm_width, realm_height, building_position + Position(1, 0));
				game->addRealm(realm_id, new_realm);
				new_realm->remakePathMap(ChunkRange({-1, -1}, {1, 1}));
				realm->add(building);
			};

			while (!buildable_set.empty()) {
				building_position = *buildable_set.begin();
				switch (rng() % 8) {
					case 0: {
						static const std::array blacksmiths {"base:tile/blacksmith1"_id, "base:tile/blacksmith2"_id, "base:tile/blacksmith3"_id};
						gen_building(choose(blacksmiths), 9, 9, "base:realm/blacksmith", WorldGen::generateBlacksmith);
						break;
					}

					case 1: {
						static const std::array taverns {"base:tile/tavern1"_id};
						constexpr size_t tavern_width  = 25;
						constexpr size_t tavern_height = 15;
						gen_building(choose(taverns), tavern_width, tavern_height, "base:realm/tavern", WorldGen::generateTavern, Position(tavern_height - 2, tavern_width / 2));
						break;
					}

					case 2: {
						static const std::array arcades{"base:tile/arcade1"_id};
						constexpr size_t arcade_width  = 8;
						constexpr size_t arcade_height = 8;
						gen_building(choose(arcades), arcade_width, arcade_height, "base:realm/arcade", WorldGen::generateArcade, Position(arcade_height - 2, arcade_width / 2));
						break;
					}

					default: {
						static const std::array houses{"base:tile/house1"_id, "base:tile/house2"_id, "base:tile/house3"_id};
						gen_building(choose(houses), 9, 9, "base:realm/house", WorldGen::generateHouse);
						break;
					}
				}

				buildable_set.erase(building_position);
				// Some of these are sus if building_position happens to be at the west or east edge, but those aren't valid locations for houses anyway.
				buildable_set.erase(building_position - Position(1, 0));
				buildable_set.erase(building_position + Position(1, 0));
				buildable_set.erase(building_position - Position(1, 1));
				buildable_set.erase(building_position + Position(1, -1));
				buildable_set.erase(building_position + Position(-1, 1));
				buildable_set.erase(building_position + Position(1, 1));
				buildable_set.erase(building_position - Position(0, 1));
				buildable_set.erase(building_position + Position(0, 1));
			}
		}

		houses_timer.stop();
	}
}
