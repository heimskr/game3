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

		const auto cleanup = [&](Index row, Index column) {
			if (auto tile_entity = realm->tileEntityAt({row, column}))
				realm->remove(tile_entity);
		};

		const auto set1 = [&](const Identifier &tilename) {
			cleanup(row, column);
			realm->setTile(1, {row, column}, tilename, false);
		};

		const auto set2 = [&](const Identifier &tilename) {
			cleanup(row, column);
			realm->setTile(2, {row, column}, tilename, false);
		};

		const auto set2p = [&](const Position &position, const Identifier &tilename) {
			cleanup(position.row, position.column);
			realm->setTile(2, position, tilename, false);
		};

		Timer town_timer("TownLayout");
		for (row = position.row - pad; row < position.row + height + pad; ++row)
			for (column = position.column - pad; column < position.column + width + pad; ++column)
				set2p({row, column}, "base:tile/empty"_id);

		for (row = position.row; row < position.row + height; ++row) {
			set2p({row, position.column}, "base:tile/tower_ns"_id);
			set2p({row, position.column + width - 1}, "base:tile/tower_ns"_id);
		}

		for (column = position.column; column < position.column + width; ++column) {
			set2p({position.row, column}, "base:tile/tower_we"_id);
			set2p({position.row + height - 1, column}, "base:tile/tower_we"_id); // TODO!: verify
		}

		set2p(position, "base:tile/tower_nw"_id);
		set2p({position.row + height - 1, position.column}, "base:tile/tower_sw"_id);
		set2p({position.row, position.column + width - 1}, "base:tile/tower_ne"_id);
		set2p({position.row + height - 1, position.column + width - 1}, "base:tile/tower_se"_id);

		std::unordered_set<Position> buildable_set;

		for (row = position.row + 1; row < position.row + height - 1; ++row)
			for (column = position.column + 1; column < position.column + width - 1; ++column) {
				buildable_set.insert({row, column});
				set1("base:tile/dirt"_id);
			}

		row = position.row + height / 2 - 1;
		for (column = position.column - pad; column < position.column + width + pad; ++column) {
			buildable_set.erase({row, column});
			buildable_set.erase({position.row + height - 2,  column}); // Make sure no houses spawn on the bottom row of the town
			set1("base:tile/road"_id);
			++row;
			buildable_set.erase({row, column});
			set1("base:tile/road"_id);
			--row;
		}
		column = position.column;
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
		column = position.column + width / 2 - 1;
		for (row = position.row - pad; row < position.row + height + pad; ++row) {
			buildable_set.erase({row, column});
			set1("base:tile/road"_id);
			++column;
			buildable_set.erase({row, column});
			set1("base:tile/road"_id);
			--column;
		}
		row = position.row;
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

		Position keep_position(position.row + height / 2 - 1, position.column + width / 2 - 1);
		const Position town_origin(position.row - pad, position.column - pad);
		const RealmID keep_realm_id = game.newRealmID();
		const Index keep_width = 15;
		const Index keep_height = 15;
		const Position keep_entrance(keep_height - 1, keep_width / 2);
		const Position keep_exit = keep_position + Position(2, 0);
		auto keep_tileset = game.registry<TilesetRegistry>().at("base:tileset/monomap");
		auto keep_tilemap = std::make_shared<Tilemap>(keep_width, keep_height, 16, keep_tileset);
		auto keep_biomemap = std::make_shared<BiomeMap>(keep_width, keep_height);
		auto keep_realm = Realm::create<Keep>(game, keep_realm_id, town_origin, width, height, -seed);
		keep_realm->outdoors = false;
		WorldGen::generateKeep(keep_realm, rng, realm->id, width, height, keep_exit);
		game.realms.emplace(keep_realm_id, keep_realm);

		auto create_keep = [&](const Identifier &tilename) {
			realm->setTile(2, keep_position, tilename);
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
				realm->setTile(2, building_position, tilename);
				const RealmID realm_id = game.newRealmID();
				// auto building = TileEntity::create<Building>(game, tilename, building_position, realm_id, entrance == -1? realm_width * (realm_height - 1) - 3 : entrance);
				auto building = TileEntity::create<Building>(game, tilename, building_position, realm_id, entrance? *entrance : Position(realm_height - 2, realm_width - 3)); // TODO!: verify
				auto details = game.registry<RealmDetailsRegistry>()[realm_type];
				auto new_realm = Realm::create(game, realm_id, realm_type, details->tilesetName, -seed);
				new_realm->outdoors = false;
				gen_fn(new_realm, rng, realm, realm_width, realm_height, building_position + Position(1, 0));
				game.realms.emplace(realm_id, new_realm);
				realm->add(building);
			};

			while (!buildable_set.empty()) {
				building_position = *buildable_set.begin();
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
						gen_building(choose(taverns), tavern_width, tavern_height, "base:realm/tavern"_id, WorldGen::generateTavern, Position(tavern_height - 2, tavern_width / 2)); // TODO!: verify
						//tavern_width * (tavern_height - 2) + tavern_width / 2);
						break;
					}

					default: {
						static std::array<Identifier, 3> houses {"base:tile/house1"_id, "base:tile/house2"_id, "base:tile/house3"_id};
						gen_building(choose(houses), 9, 9, "base:realm/house"_id, WorldGen::generateHouse);
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
