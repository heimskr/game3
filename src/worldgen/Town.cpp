#include "Tileset.h"
#include "game/Game.h"
#include "realm/Keep.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/BlacksmithGen.h"
#include "worldgen/Keep.h"
#include "worldgen/House.h"
#include "worldgen/Tavern.h"
#include "worldgen/WorldGen.h"

namespace Game3::WorldGen {
	void generateTown(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, const Position &position, Index width, Index height, Index pad, int seed) {
		Index row = 0;
		Index column = 0;

		Game &game = realm->getGame();
		const auto map_width = realm->getWidth();

		const auto cleanup = [&](Index row, Index column) {
			if (auto tile_entity = realm->tileEntityAt({row, column}))
				realm->remove(tile_entity);
		};

		const auto set1 = [&](const Identifier &tilename) {
			cleanup(row, column);
			realm->setLayer1({row, column}, tilename);
		};

		const auto set2 = [&](const Identifier &tilename) {
			cleanup(row, column);
			realm->setLayer2({row, column}, tilename);
		};

		const auto set2i = [&](Index index, const Identifier &tilename) {
			auto [r, c] = realm->getPosition(index);
			cleanup(r, c);
			realm->setLayer2(index, tilename);
		};

		Timer town_timer("TownLayout");
		for (row = index / map_width - pad; row < index / map_width + height + pad; ++row)
			for (column = index % map_width - pad; column < index % map_width + width + pad; ++column)
				set2i(row * map_width + column, "base:tile/empty"_id);

		for (row = index / map_width; row < index / map_width + height; ++row) {
			set2i(row * map_width + index % map_width, "base:tile/tower_ns"_id);
			set2i(row * map_width + index % map_width + width - 1, "base:tile/tower_ns"_id);
		}

		for (column = 0; column < width; ++column) {
			set2i(index + column, "base:tile/tower_we"_id);
			set2i(index + map_width * (height - 1) + column, "base:tile/tower_we"_id);
		}

		set2i(index, "base:tile/tower_nw"_id);
		set2i(index + map_width * (height - 1), "base:tile/tower_sw"_id);
		set2i(index + width - 1, "base:tile/tower_ne"_id);
		set2i(index + map_width * (height - 1) + width - 1, "base:tile/tower_se"_id);

		std::unordered_set<Index> buildable_set;

		for (row = index / map_width + 1; row < index / map_width + height - 1; ++row)
			for (column = index % map_width + 1; column < index % map_width + width - 1; ++column) {
				buildable_set.insert(row * map_width + column);
				set1("base:tile/dirt"_id);
			}

		row = index / map_width + height / 2 - 1;
		for (column = index % map_width - pad; column < index % map_width + width + pad; ++column) {
			buildable_set.erase(row * map_width + column);
			buildable_set.erase((index / map_width + height - 2) * map_width + column); // Make sure no houses spawn on the bottom row of the town
			set1("base:tile/road"_id);
			++row;
			buildable_set.erase(row * map_width + column);
			set1("base:tile/road"_id);
			--row;
		}
		column = index % map_width;
		set2("base:tile/empty"_id);
		--row;
		set2("base:tile/tower_s"_id);
		row += 2;
		set2("base:tile/empty"_id);
		++row;
		set2("base:tile/tower_n"_id);
		--row;
		column += width - 1;
		set2("base:tile/empty"_id);
		--row;
		set2("base:tile/empty"_id);
		--row;
		set2("base:tile/tower_s"_id);
		row += 3;
		set2("base:tile/tower_n"_id);
		--row;
		column = index % map_width + width / 2 - 1;
		for (row = index / map_width - pad; row < index / map_width + height + pad; ++row) {
			buildable_set.erase(row * map_width + column);
			set1("base:tile/road"_id);
			++column;
			buildable_set.erase(row * map_width + column);
			set1("base:tile/road"_id);
			--column;
		}
		row = index / map_width;
		set2("base:tile/empty"_id);
		--column;
		set2("base:tile/tower_ne"_id);
		column += 2;
		set2("base:tile/empty"_id);
		++column;
		set2("base:tile/tower_nw"_id);
		column -= 2;
		row += height - 1;
		set2("base:tile/empty"_id);
		--column;
		set2("base:tile/tower_ne"_id);
		column += 2;
		set2("base:tile/empty"_id);
		++column;
		set2("base:tile/tower_nw"_id);
		--column;

		Position keep_position(index / map_width + height / 2 - 1, index % map_width + width / 2 - 1);
		const Position town_origin(index / map_width - pad, index % map_width - pad);
		const RealmID keep_realm_id = game.newRealmID();
		const Index keep_width = 15;
		const Index keep_height = 15;
		const Index keep_entrance = keep_width * (keep_height - 1) - keep_width / 2 - 1;
		const Position keep_exit = keep_position + Position(2, 0);
		auto keep_tileset = game.registry<TilesetRegistry>().at("base:tileset/monomap");
		auto keep_tilemap = std::make_shared<Tilemap>(keep_width, keep_height, 16, keep_tileset);
		auto keep_biomemap = std::make_shared<BiomeMap>(keep_width, keep_height);
		auto keep_realm = Realm::create<Keep>(game, keep_realm_id, town_origin, width, height, keep_tilemap, keep_biomemap, -seed);
		keep_realm->outdoors = false;
		WorldGen::generateKeep(keep_realm, rng, realm->id, keep_exit);
		game.realms.emplace(keep_realm_id, keep_realm);

		auto create_keep = [&](const Identifier &tilename) {
			realm->setLayer2(keep_position, tilename);
			realm->add(TileEntity::create<Building>(game, tilename, keep_position, keep_realm_id, keep_entrance));
		};

		create_keep("base:tile/keep_nw"_id);
		++keep_position.column;
		create_keep("base:tile/keep_ne"_id);
		++keep_position.row;
		create_keep("base:tile/keep_se"_id);
		--keep_position.column;
		create_keep("base:tile/keep_sw"_id);

		// Prevent houses from being placed on the corners around the keep
		buildable_set.erase(map_width * (index / map_width + height / 2 - 2) + index % map_width + width / 2 - 2);
		buildable_set.erase(map_width * (index / map_width + height / 2 - 2) + index % map_width + width / 2 + 1);
		buildable_set.erase(map_width * (index / map_width + height / 2 + 1) + index % map_width + width / 2 + 1);
		buildable_set.erase(map_width * (index / map_width + height / 2 + 1) + index % map_width + width / 2 - 2);

		town_timer.stop();
		std::vector<Index> buildable(buildable_set.cbegin(), buildable_set.cend());
		std::shuffle(buildable.begin(), buildable.end(), rng);
		Timer houses_timer("Houses");

		if (2 < buildable.size()) {
			buildable.erase(buildable.begin() + buildable.size() / 10, buildable.end());
			buildable_set = std::unordered_set<Index>(buildable.cbegin(), buildable.cend());

			Index building_index = 0;

			auto gen_building = [&](const Identifier &tilename, Index realm_width, Index realm_height, RealmType realm_type, const BuildingGenerator &gen_fn, Index entrance = -1) {
				realm->setLayer2(building_index, tilename);
				const RealmID realm_id = game.newRealmID();
				const Position building_position(building_index / map_width, building_index % map_width);
				auto building = TileEntity::create<Building>(game, tilename, building_position, realm_id, entrance == -1? realm_width * (realm_height - 1) - 3 : entrance);
				auto details = game.registry<RealmDetailsRegistry>()[realm_type];
				auto new_tileset = game.registry<TilesetRegistry>()[details->tilesetName];
				auto new_tilemap = std::make_shared<Tilemap>(realm_width, realm_height, 16, new_tileset);
				auto new_biomemap = std::make_shared<BiomeMap>(realm_width, realm_height);
				auto new_realm = Realm::create(game, realm_id, realm_type, new_tilemap, new_biomemap, -seed);
				new_realm->outdoors = false;
				gen_fn(new_realm, rng, realm, building_position + Position(1, 0));
				game.realms.emplace(realm_id, new_realm);
				realm->add(building);
			};

			while (!buildable_set.empty()) {
				building_index = *buildable_set.begin();
				switch (rng() % 8) {
					case 0: {
						static std::array<Identifier, 3> blacksmiths {"base:tile/blacksmith1"_id, "base:tile/blacksmith2"_id, "base:tile/blacksmith3"_id};
						gen_building(choose(blacksmiths), 9, 9, "base:realm/blacksmith"_id, WorldGen::generateBlacksmith);
						break;
					}

					case 1: {
						static std::array<Identifier, 1> taverns {"base:tile/tavern1"_id};
						constexpr size_t tavern_width  = 25;
						constexpr size_t tavern_height = 15;
						gen_building(choose(taverns), tavern_width, tavern_height, "base:realm/tavern"_id, WorldGen::generateTavern, tavern_width * (tavern_height - 2) + tavern_width / 2);
						break;
					}

					default: {
						static std::array<Identifier, 3> houses {"base:tile/house1"_id, "base:tile/house2"_id, "base:tile/house3"_id};
						gen_building(choose(houses), 9, 9, "base:realm/house"_id, WorldGen::generateHouse);
						break;
					}
				}

				buildable_set.erase(building_index);
				// Some of these are sus if building_index happens to be at the west or east edge, but those aren't valid locations for houses anyway.
				buildable_set.erase(building_index - map_width);
				buildable_set.erase(building_index + map_width);
				buildable_set.erase(building_index - map_width - 1);
				buildable_set.erase(building_index + map_width - 1);
				buildable_set.erase(building_index - map_width + 1);
				buildable_set.erase(building_index + map_width + 1);
				buildable_set.erase(building_index - 1);
				buildable_set.erase(building_index + 1);
			}
		}

		houses_timer.stop();
	}
}
