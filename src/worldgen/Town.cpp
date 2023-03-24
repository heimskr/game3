#include "Tiles.h"
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
	void generateTown(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, const Index index, Index width, Index height, Index pad, int seed) {
		Index row = 0;
		Index column = 0;

		Game &game = realm->getGame();
		const auto map_width = realm->getWidth();

		const auto cleanup = [&](Index row, Index column) {
			static size_t count = 0;
			if (auto tile_entity = realm->tileEntityAt({row, column}))
				realm->remove(tile_entity);
		};

		const auto set1 = [&](TileID tile) {
			cleanup(row, column);
			realm->setLayer1(row, column, tile);
		};

		const auto set2 = [&](TileID tile) {
			cleanup(row, column);
			realm->setLayer2(row, column, tile);
		};

		const auto set2i = [&](Index index, TileID tile) {
			auto [r, c] = realm->getPosition(index);
			cleanup(r, c);
			realm->setLayer2(index, tile);
		};

		Timer town_timer("TownLayout");
		for (row = index / map_width - pad; row < index / map_width + height + pad; ++row)
			for (column = index % map_width - pad; column < index % map_width + width + pad; ++column)
				set2i(row * map_width + column, Monomap::EMPTY);

		for (row = index / map_width; row < index / map_width + height; ++row) {
			set2i(row * map_width + index % map_width, Monomap::TOWER_NS);
			set2i(row * map_width + index % map_width + width - 1, Monomap::TOWER_NS);
		}

		for (column = 0; column < width; ++column) {
			set2i(index + column, Monomap::TOWER_WE);
			set2i(index + map_width * (height - 1) + column, Monomap::TOWER_WE);
		}

		set2i(index, Monomap::TOWER_NW);
		set2i(index + map_width * (height - 1), Monomap::TOWER_SW);
		set2i(index + width - 1, Monomap::TOWER_NE);
		set2i(index + map_width * (height - 1) + width - 1, Monomap::TOWER_SE);

		std::unordered_set<Index> buildable_set;

		for (row = index / map_width + 1; row < index / map_width + height - 1; ++row)
			for (column = index % map_width + 1; column < index % map_width + width - 1; ++column) {
				buildable_set.insert(row * map_width + column);
				set1(Monomap::DIRT);
			}

		row = index / map_width + height / 2 - 1;
		for (column = index % map_width - pad; column < index % map_width + width + pad; ++column) {
			buildable_set.erase(row * map_width + column);
			buildable_set.erase((index / map_width + height - 2) * map_width + column); // Make sure no houses spawn on the bottom row of the town
			set1(Monomap::ROAD);
			++row;
			buildable_set.erase(row * map_width + column);
			set1(Monomap::ROAD);
			--row;
		}
		column = index % map_width;
		set2(Monomap::EMPTY);
		--row;
		set2(Monomap::TOWER_S);
		row += 2;
		set2(Monomap::EMPTY);
		++row;
		set2(Monomap::TOWER_N);
		--row;
		column += width - 1;
		set2(Monomap::EMPTY);
		--row;
		set2(Monomap::EMPTY);
		--row;
		set2(Monomap::TOWER_S);
		row += 3;
		set2(Monomap::TOWER_N);
		--row;
		column = index % map_width + width / 2 - 1;
		for (row = index / map_width - pad; row < index / map_width + height + pad; ++row) {
			buildable_set.erase(row * map_width + column);
			set1(Monomap::ROAD);
			++column;
			buildable_set.erase(row * map_width + column);
			set1(Monomap::ROAD);
			--column;
		}
		row = index / map_width;
		set2(Monomap::EMPTY);
		--column;
		set2(Monomap::TOWER_NE);
		column += 2;
		set2(Monomap::EMPTY);
		++column;
		set2(Monomap::TOWER_NW);
		column -= 2;
		row += height - 1;
		set2(Monomap::EMPTY);
		--column;
		set2(Monomap::TOWER_NE);
		column += 2;
		set2(Monomap::EMPTY);
		++column;
		set2(Monomap::TOWER_NW);
		--column;

		Position keep_position(index / map_width + height / 2 - 1, index % map_width + width / 2 - 1);
		const Position town_origin(index / map_width - pad, index % map_width - pad);
		const RealmID keep_realm_id = game.newRealmID();
		const Index keep_width = 15;
		const Index keep_height = 15;
		const Index keep_entrance = keep_width * (keep_height - 1) - keep_width / 2 - 1;
		const Position keep_exit = keep_position + Position(2, 0);
		auto keep_tilemap = std::make_shared<Tilemap>(keep_width, keep_height, 16, Realm::textureMap.at(Realm::KEEP));
		auto keep_realm = Realm::create<Keep>(keep_realm_id, town_origin, width, height, keep_tilemap, -seed);
		keep_realm->outdoors = false;
		keep_realm->setGame(game);
		WorldGen::generateKeep(keep_realm, rng, realm->id, keep_exit);
		game.realms.emplace(keep_realm_id, keep_realm);

		auto create_keep = [&](TileID tile) {
			realm->setLayer2(keep_position, tile);
			realm->add(TileEntity::create<Building>(tile, keep_position, keep_realm_id, keep_entrance));
		};

		create_keep(Monomap::KEEP_NW);
		++keep_position.column;
		create_keep(Monomap::KEEP_NE);
		++keep_position.row;
		create_keep(Monomap::KEEP_SE);
		--keep_position.column;
		create_keep(Monomap::KEEP_SW);

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

			auto gen_building = [&](TileID tile_id, Index realm_width, Index realm_height, RealmType realm_type, const BuildingGenerator &gen_fn, Index entrance = -1) {
				realm->setLayer2(building_index, tile_id);
				const RealmID realm_id = game.newRealmID();
				const Position building_position(building_index / map_width, building_index % map_width);
				auto building = TileEntity::create<Building>(tile_id, building_position, realm_id, entrance == -1? realm_width * (realm_height - 1) - 3 : entrance);
				auto new_tilemap = std::make_shared<Tilemap>(realm_width, realm_height, 16, Realm::textureMap.at(realm_type));
				auto new_realm = Realm::create(realm_id, realm_type, new_tilemap, -seed);
				new_realm->outdoors = false;
				new_realm->setGame(game);
				gen_fn(new_realm, rng, realm, building_position + Position(1, 0));
				game.realms.emplace(realm_id, new_realm);
				realm->add(building);
			};

			while (!buildable_set.empty()) {
				building_index = *buildable_set.begin();
				switch (rng() % 8) {
					case 0: {
						constexpr static std::array<TileID, 3> blacksmiths {Monomap::BLACKSMITH1, Monomap::BLACKSMITH2, Monomap::BLACKSMITH3};
						gen_building(choose(blacksmiths), 9, 9, Realm::BLACKSMITH, WorldGen::generateBlacksmith);
						break;
					}

					case 1: {
						constexpr static std::array<TileID, 1> taverns {Monomap::TAVERN1};
						constexpr size_t tavern_width  = 25;
						constexpr size_t tavern_height = 15;
						gen_building(choose(taverns), tavern_width, tavern_height, Realm::TAVERN, WorldGen::generateTavern, tavern_width * (tavern_height - 2) + tavern_width / 2);
						break;
					}

					default: {
						constexpr static std::array<TileID, 3> houses {Monomap::HOUSE1, Monomap::HOUSE2, Monomap::HOUSE3};
						gen_building(choose(houses), 9, 9, Realm::HOUSE, WorldGen::generateHouse);
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
