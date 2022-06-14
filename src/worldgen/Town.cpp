#include "Tiles.h"
#include "game/Game.h"
#include "realm/Keep.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Blacksmith.h"
#include "worldgen/Keep.h"
#include "worldgen/House.h"

namespace Game3::WorldGen {
	void generateTown(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, const Index index, Index width, Index height, Index pad) {
		Index row = 0;
		Index column = 0;

		Game &game = realm->getGame();
		const auto map_width = realm->getWidth();

		const auto set1 = [&](TileID tile) { realm->setLayer1(row, column, tile); };
		const auto set2 = [&](TileID tile) { realm->setLayer2(row, column, tile); };

		Timer town_timer("TownLayout");
		for (row = index / map_width - pad; row < index / map_width + height + pad; ++row)
			for (column = index % map_width - pad; column < index % map_width + width + pad; ++column)
				realm->setLayer2(row * map_width + column, OverworldTiles::EMPTY);

		for (row = index / map_width; row < index / map_width + height; ++row) {
			realm->setLayer2(row * map_width + index % map_width, OverworldTiles::TOWER_NS);
			realm->setLayer2(row * map_width + index % map_width + width - 1, OverworldTiles::TOWER_NS);
		}

		for (column = 0; column < width; ++column) {
			realm->setLayer2(index + column, OverworldTiles::TOWER_WE);
			realm->setLayer2(index + map_width * (height - 1) + column, OverworldTiles::TOWER_WE);
		}

		realm->setLayer2(index, OverworldTiles::TOWER_NW);
		realm->setLayer2(index + map_width * (height - 1), OverworldTiles::TOWER_SW);
		realm->setLayer2(index + width - 1, OverworldTiles::TOWER_NE);
		realm->setLayer2(index + map_width * (height - 1) + width - 1, OverworldTiles::TOWER_SE);

		std::unordered_set<Index> buildable_set;

		for (row = index / map_width + 1; row < index / map_width + height - 1; ++row)
			for (column = index % map_width + 1; column < index % map_width + width - 1; ++column) {
				buildable_set.insert(row * map_width + column);
				set1(OverworldTiles::DIRT);
				if (auto tile_entity = realm->tileEntityAt({row, column}))
					realm->remove(tile_entity);
			}

		row = index / map_width + height / 2 - 1;
		for (column = index % map_width - pad; column < index % map_width + width + pad; ++column) {
			buildable_set.erase(row * map_width + column);
			buildable_set.erase((index / map_width + height - 2) * map_width + column); // Make sure no houses spawn on the bottom row of the town
			set1(OverworldTiles::ROAD);
			++row;
			buildable_set.erase(row * map_width + column);
			set1(OverworldTiles::ROAD);
			--row;
		}
		column = index % map_width;
		set2(OverworldTiles::EMPTY);
		--row;
		set2(OverworldTiles::TOWER_S);
		row += 2;
		set2(OverworldTiles::EMPTY);
		++row;
		set2(OverworldTiles::TOWER_N);
		--row;
		column += width - 1;
		set2(OverworldTiles::EMPTY);
		--row;
		set2(OverworldTiles::EMPTY);
		--row;
		set2(OverworldTiles::TOWER_S);
		row += 3;
		set2(OverworldTiles::TOWER_N);
		--row;
		column = index % map_width + width / 2 - 1;
		for (row = index / map_width - pad; row < index / map_width + height + pad; ++row) {
			buildable_set.erase(row * map_width + column);
			set1(OverworldTiles::ROAD);
			++column;
			buildable_set.erase(row * map_width + column);
			set1(OverworldTiles::ROAD);
			--column;
		}
		row = index / map_width;
		set2(OverworldTiles::EMPTY);
		--column;
		set2(OverworldTiles::TOWER_NE);
		column += 2;
		set2(OverworldTiles::EMPTY);
		++column;
		set2(OverworldTiles::TOWER_NW);
		column -= 2;
		row += height - 1;
		set2(OverworldTiles::EMPTY);
		--column;
		set2(OverworldTiles::TOWER_NE);
		column += 2;
		set2(OverworldTiles::EMPTY);
		++column;
		set2(OverworldTiles::TOWER_NW);
		--column;

		Position keep_position(index / map_width + height / 2 - 1, index % map_width + width / 2 - 1);
		const Position town_origin(index / map_width - pad, index % map_width - pad);
		const RealmID keep_realm_id = game.newRealmID();
		const Index keep_width = 15;
		const Index keep_height = 15;
		const Index keep_entrance = keep_width * (keep_height - 1) - keep_width / 2 - 1;
		const Position keep_exit = keep_position + Position(2, 0);
		auto keep_tilemap = std::make_shared<Tilemap>(keep_width, keep_height, 16, Realm::textureMap.at(Realm::KEEP));
		auto keep_realm = Realm::create<Keep>(keep_realm_id, town_origin, width, height, keep_tilemap);
		keep_realm->outdoors = false;
		keep_realm->setGame(game);
		WorldGen::generateKeep(keep_realm, rng, realm->id, keep_exit);
		game.realms.emplace(keep_realm_id, keep_realm);

		auto create_keep = [&](TileID tile) {
			realm->setLayer2(keep_position, tile);
			realm->add(TileEntity::create<Building>(tile, keep_position, keep_realm_id, keep_entrance));
		};

		create_keep(OverworldTiles::KEEP_NW);
		++keep_position.column;
		create_keep(OverworldTiles::KEEP_NE);
		++keep_position.row;
		create_keep(OverworldTiles::KEEP_SE);
		--keep_position.column;
		create_keep(OverworldTiles::KEEP_SW);

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
			while (!buildable_set.empty()) {
				const auto index = *buildable_set.begin();
				if (rng() % 8 == 0) {
					constexpr static std::array<TileID, 3> blacksmiths {OverworldTiles::BLACKSMITH1, OverworldTiles::BLACKSMITH2, OverworldTiles::BLACKSMITH3};
					const auto blacksmith = choose(blacksmiths, rng);
					realm->setLayer2(index, blacksmith);
					const RealmID realm_id = game.newRealmID();
					const Index realm_width  = 9;
					const Index realm_height = 9;
					const Position blacksmith_position {index / map_width, index % map_width};
					auto building = TileEntity::create<Building>(blacksmith, blacksmith_position, realm_id, realm_width * (realm_height - 1) - 3);
					auto new_tilemap = std::make_shared<Tilemap>(realm_width, realm_height, 16, Realm::textureMap.at(Realm::HOUSE));
					auto new_realm = Realm::create(realm_id, Realm::BLACKSMITH, new_tilemap);
					new_realm->outdoors = false;
					new_realm->setGame(game);
					WorldGen::generateBlacksmith(new_realm, rng, realm, blacksmith_position + Position(1, 0));
					game.realms.emplace(realm_id, new_realm);
					realm->add(building);
				} else {
					constexpr static std::array<TileID, 3> houses {OverworldTiles::HOUSE1, OverworldTiles::HOUSE2, OverworldTiles::HOUSE3};
					const auto house = choose(houses, rng);
					realm->setLayer2(index, house);
					const RealmID realm_id = game.newRealmID();
					const Index realm_width  = 9;
					const Index realm_height = 9;
					const Position house_position {index / map_width, index % map_width};
					auto building = TileEntity::create<Building>(house, house_position, realm_id, realm_width * (realm_height - 1) - 3);
					auto new_tilemap = std::make_shared<Tilemap>(realm_width, realm_height, 16, Realm::textureMap.at(Realm::HOUSE));
					auto new_realm = Realm::create(realm_id, Realm::HOUSE, new_tilemap);
					new_realm->outdoors = false;
					new_realm->setGame(game);
					WorldGen::generateHouse(new_realm, rng, realm, house_position + Position(1, 0));
					game.realms.emplace(realm_id, new_realm);
					realm->add(building);
				}

				buildable_set.erase(index);
				// Some of these are sus if index happens to be at the west or east edge, but those aren't valid locations for houses anyway.
				buildable_set.erase(index - map_width);
				buildable_set.erase(index + map_width);
				buildable_set.erase(index - map_width - 1);
				buildable_set.erase(index + map_width - 1);
				buildable_set.erase(index - map_width + 1);
				buildable_set.erase(index + map_width + 1);
				buildable_set.erase(index - 1);
				buildable_set.erase(index + 1);
			}
		}

		houses_timer.stop();
	}
}
