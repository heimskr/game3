#include <iostream>
#include <thread>
#include <unordered_set>

#include <libnoise/noise.h>

#include "Tiles.h"
#include "entity/Entity.h"
#include "game/Game.h"
#include "game/Realm.h"
#include "tileentity/Building.h"
#include "tileentity/Chest.h"
#include "tileentity/Sign.h"
#include "tileentity/Teleporter.h"
#include "util/Timer.h"
#include "util/Util.h"

namespace Game3 {
	std::unordered_map<RealmType, Texture> Realm::textureMap {
		{Realm::OVERWORLD, Texture("resources/tileset2.png")},
		{Realm::HOUSE,     Texture("resources/house/house.png")},
	};

	Realm::Realm(RealmID id_, RealmType type_, const std::shared_ptr<Tilemap> &tilemap1_, const std::shared_ptr<Tilemap> &tilemap2_, const std::shared_ptr<Tilemap> &tilemap3_):
	id(id_), type(type_), tilemap1(tilemap1_), tilemap2(tilemap2_), tilemap3(tilemap3_) {
		if (tilemap1) {
			tilemap1->init();
			renderer1.init(tilemap1);
		}

		if (tilemap2) {
			tilemap2->init();
			renderer2.init(tilemap2);
		}

		if (tilemap3) {
			tilemap3->init();
			renderer3.init(tilemap3);
		}
	}

	Realm::Realm(RealmID id_, RealmType type_, const std::shared_ptr<Tilemap> &tilemap1_): Realm(id_, type_, tilemap1_, nullptr, nullptr) {
		tilemap1->init();
		tilemap2 = std::make_shared<Tilemap>(tilemap1->width, tilemap1->height, tilemap1->tileSize, tilemap1->texture);
		tilemap3 = std::make_shared<Tilemap>(tilemap1->width, tilemap1->height, tilemap1->tileSize, tilemap1->texture);
		renderer2.init(tilemap2);
		renderer3.init(tilemap3);
	}

	std::shared_ptr<Realm> Realm::fromJSON(const nlohmann::json &json) {
		auto out = Realm::create();
		auto &realm = *out;
		realm.id = json.at("id");
		realm.type = json.at("type");
		realm.tilemap1 = std::make_shared<Tilemap>(json.at("tilemap1"));
		realm.tilemap2 = std::make_shared<Tilemap>(json.at("tilemap2"));
		realm.tilemap3 = std::make_shared<Tilemap>(json.at("tilemap3"));
		realm.tilemap1->texture.init();
		realm.tilemap2->texture.init();
		realm.tilemap3->texture.init();
		for (const auto &[index, tile_entity_json]: json.at("tileEntities").get<std::unordered_map<std::string, nlohmann::json>>())
			realm.tileEntities.emplace(parseUlong(index), TileEntity::fromJSON(tile_entity_json)).first->second->setRealm(out);
		realm.renderer1.init(realm.tilemap1);
		realm.renderer2.init(realm.tilemap2);
		realm.renderer3.init(realm.tilemap3);
		realm.entities.clear();
		for (const auto &entity_json: json.at("entities"))
			(*realm.entities.insert(Entity::fromJSON(entity_json)).first)->setRealm(out);
		return out;
	}

	void Realm::render(const int width, const int height, const nanogui::Vector2f &center, float scale, SpriteRenderer &sprite_renderer) {
		renderer1.center = center;
		renderer1.scale  = scale;
		renderer1.onBackbufferResized(width, height);
		renderer2.center = center;
		renderer2.scale  = scale;
		renderer2.onBackbufferResized(width, height);
		renderer3.center = center;
		renderer3.scale  = scale;
		renderer3.onBackbufferResized(width, height);
		renderer1.render();
		renderer2.render();
		renderer3.render();
		for (const auto &entity: entities)
			entity->render(sprite_renderer);
		for (const auto &[index, tile_entity]: tileEntities)
			tile_entity->render(sprite_renderer);
	}

	void Realm::reupload() {
		renderer1.reupload();
		renderer2.reupload();
		renderer3.reupload();
	}

	void Realm::rebind() {
		renderer1.tilemap = tilemap1;
		renderer2.tilemap = tilemap2;
		renderer3.tilemap = tilemap3;
	}

	void Realm::generate(int seed, double noise_zoom, double noise_threshold) {
		if (game == nullptr)
			throw std::runtime_error("Can't call Realm::generate when game is null");

		const auto width = tilemap1->width;
		const auto height = tilemap1->height;

		noise::module::Perlin perlin;
		perlin.SetSeed(seed);

		auto &tiles1 = tilemap1->tiles;
		tiles1.assign(tiles1.size(), 0);
		tilemap2->tiles.assign(tilemap2->tiles.size(), 0);
		tilemap3->tiles.assign(tilemap3->tiles.size(), 0);

		static const std::vector<TileID> grasses {
			OverworldTiles::GRASS_ALT1, OverworldTiles::GRASS_ALT2,
			OverworldTiles::GRASS, OverworldTiles::GRASS, OverworldTiles::GRASS, OverworldTiles::GRASS, OverworldTiles::GRASS, OverworldTiles::GRASS, OverworldTiles::GRASS
		};

		std::default_random_engine grass_rng;
		grass_rng.seed(seed);

		for (int row = 0; row < width; ++row)
			for (int column = 0; column < height; ++column) {
				double noise = perlin.GetValue(row / noise_zoom, column / noise_zoom, 0.666);
				auto &tile = tiles1[column * width + row];
				if (noise < noise_threshold)
					tile = OverworldTiles::DEEPER_WATER;
				else if (noise < noise_threshold + 0.1)
					tile = OverworldTiles::DEEP_WATER;
				else if (noise < noise_threshold + 0.2)
					tile = OverworldTiles::WATER;
				else if (noise < noise_threshold + 0.3)
					tile = OverworldTiles::SHALLOW_WATER;
				else if (noise < noise_threshold + 0.4)
					tile = OverworldTiles::SAND;
				else if (noise < noise_threshold + 0.5)
					tile = OverworldTiles::LIGHT_GRASS;
				else
					tile = choose(grasses, grass_rng);
			}

		constexpr static int m = 15, n = 21, pad = 2;
		Timer land_timer("GetLand");
		auto starts = tilemap1->getLand(type, m + pad * 2, n + pad * 2);
		land_timer.stop();
		randomLand = choose(starts, uint_fast32_t(seed));
		std::vector<Index> candidates;
		candidates.reserve(starts.size() / 16);
		Timer candidate_timer("Candidates");
		for (const auto index: starts) {
			const size_t row_start = index / tilemap1->width + pad, row_end = row_start + m;
			const size_t column_start = index % tilemap1->width + pad, column_end = column_start + n;
			for (size_t row = row_start; row < row_end; ++row)
				for (size_t column = column_start; column < column_end; ++column)
					if (!overworldTiles.isLand(tiles1[row * tilemap1->width + column]))
						goto failed;
			candidates.push_back(index);
			failed:
			continue;
		}
		candidate_timer.stop();

		std::cout << "Found " << candidates.size() << " candidate" << (candidates.size() == 1? "" : "s") << ".\n";
		if (!candidates.empty())
			createTown(choose(candidates, uint_fast32_t(seed)) + pad * (tilemap1->width + 1), n, m, pad);

		Timer::summary();
		Timer::clear();
	}

	void Realm::generateHouse(RealmID parent_realm, std::default_random_engine &rng, const Position &entrance, int width, int height) {
		auto &layer1 = tilemap1->tiles;
		auto &layer2 = tilemap2->tiles;

		for (int column = 1; column < width - 1; ++column) {
			layer2[column] = HouseTiles::WALL_WEN;
			layer2[column + (height - 1) * width] = HouseTiles::WALL_WES;
		}

		for (int row = 1; row < height - 1; ++row)
			layer2[row * width] = layer2[(row + 1) * width - 1] = HouseTiles::WALL_NS;

		for (int row = 0; row < height; ++row)
			for (int column = 0; column < width; ++column)
				layer1[row * width + column] = HouseTiles::FLOOR;

		layer2[0] = HouseTiles::WALL_NW;
		layer2[width - 1] = HouseTiles::WALL_NE;
		layer2[width * (height - 1)] = HouseTiles::WALL_SW;
		layer2[width * height - 1] = HouseTiles::WALL_SE;

		const Index exit_index = width * height - 3;
		layer2[width * height - 2] = HouseTiles::WALL_E;
		layer2[width * height - 3] = HouseTiles::EMPTY;
		layer2[width * height - 4] = HouseTiles::WALL_W;

		static std::array<TileID, 3> plants {HouseTiles::PLANT1, HouseTiles::PLANT2, HouseTiles::PLANT3};
		static std::array<TileID, 2> doors  {HouseTiles::DOOR1,  HouseTiles::DOOR2};

		layer2[width + 1] = choose(plants, rng);
		layer2[2 * width - 2] = choose(plants, rng);
		layer2[(width - 1) * height - 2] = choose(plants, rng);
		layer2[(width - 2) * height + 1] = choose(plants, rng);

		add(TileEntity::create<Teleporter>(choose(doors, rng), getPosition(exit_index), parent_realm, entrance));

		switch(rng() % 2) {
			case 0: {
				const static std::array<std::string, 13> texts {
					"Express ideas directly in code.",
					"Write in ISO Standard C++.",
					"Express intent.",
					"Ideally, a program should be statically type safe.",
					"Prefer compile-time checking to run-time checking.",
					"What cannot be checked at compile time should be checkable at run time.",
					"Catch run-time errors early.",
					"Don't leak any resources.",
					"Don't waste time or space.",
					"Prefer immutable data to mutable data.",
					"Encapsulate messy constructs, rather than spreading through the code.",
					"Use supporting tools as appropriate.",
					"Use support libraries as appropriate."
				};

				auto shuffled_texts = texts;
				std::shuffle(shuffled_texts.begin(), shuffled_texts.end(), rng);

				for (Index index = width + 2; index < 2 * width - 2; ++index) {
					layer2[index] = HouseTiles::BOOKSHELF;
					add(TileEntity::create<Sign>(HouseTiles::EMPTY, getPosition(index), shuffled_texts.at((index - width - 2) % shuffled_texts.size()), "Bookshelf"));
				}
				break;
			}

			case 1: {
				auto chest = TileEntity::create<Chest>(0, getPosition(width * 3 / 2));
				chest->setInventory(4);
				add(chest);
				break;
			}

			default:
				break;
		}

		const int carpet_offset = 8 * (rng() % 3);
		const int carpet_padding = (rng() % 2) + 2;
		for (int row = carpet_padding + 1; row < height - carpet_padding - 1; ++row)
			for (int column = carpet_padding + 1; column < width - carpet_padding - 1; ++column)
				layer1[row * width + column] = HouseTiles::CARPET_C + carpet_offset;
		for (int row = carpet_padding + 1; row < height - carpet_padding - 1; ++row) {
			layer1[row * width + carpet_padding] = HouseTiles::CARPET_W + carpet_offset;
			layer1[(row + 1) * width - carpet_padding - 1] = HouseTiles::CARPET_E + carpet_offset;
		}
		for (int column = carpet_padding + 1; column < width - carpet_padding - 1; ++column) {
			layer1[carpet_padding * width + column] = HouseTiles::CARPET_N + carpet_offset;
			layer1[(height - carpet_padding - 1) * width + column] = HouseTiles::CARPET_S + carpet_offset;
		}
		layer1[carpet_padding * width + carpet_padding] = HouseTiles::CARPET_NW + carpet_offset;
		layer1[(carpet_padding + 1) * width - carpet_padding - 1] = HouseTiles::CARPET_NE + carpet_offset;
		layer1[(height - carpet_padding - 1) * width + carpet_padding] = HouseTiles::CARPET_SW + carpet_offset;
		layer1[(height - carpet_padding) * width - carpet_padding - 1] = HouseTiles::CARPET_SE + carpet_offset;
	}

	void Realm::createTown(const size_t index, size_t width, size_t height, size_t pad) {
		size_t row = 0, column = 0;

		auto &layer1 = tilemap1->tiles;
		auto &layer2 = tilemap2->tiles;

		auto map_width = tilemap1->width;

		auto set1 = [&](TileID tile) { layer1[row * map_width + column] = tile; };
		auto set2 = [&](TileID tile) { layer2[row * map_width + column] = tile; };

		for (size_t row = index / map_width; row < index / map_width + height; ++row) {
			layer2[row * map_width + index % map_width] = OverworldTiles::TOWER_NS;
			layer2[row * map_width + index % map_width + width - 1] = OverworldTiles::TOWER_NS;
		}

		for (size_t column = 0; column < width; ++column) {
			layer2[index + column] = OverworldTiles::TOWER_WE;
			layer2[index + map_width * (height - 1) + column] = OverworldTiles::TOWER_WE;
		}

		layer2[index] = OverworldTiles::TOWER_NW;
		layer2[index + map_width * (height - 1)] = OverworldTiles::TOWER_SW;
		layer2[index + width - 1] = OverworldTiles::TOWER_NE;
		layer2[index + map_width * (height - 1) + width - 1] = OverworldTiles::TOWER_SE;

		std::unordered_set<Index> buildable_set;

		for (row = index / map_width + 1; row < index / map_width + height - 1; ++row)
			for (column = index % map_width + 1; column < index % map_width + width - 1; ++column) {
				buildable_set.insert(row * map_width + column);
				set1(OverworldTiles::DIRT);
			}

		row = index / map_width + height / 2;
		for (column = index % map_width - pad; column < index % map_width + width + pad; ++column) {
			buildable_set.erase(row * map_width + column);
			buildable_set.erase((index / map_width + height - 2) * map_width + column); // Make sure no houses spawn on the bottom row of the town
			set1(OverworldTiles::ROAD);
		}
		column = index % map_width;
		set2(OverworldTiles::EMPTY);
		--row;
		set2(OverworldTiles::TOWER_S);
		row += 2;
		set2(OverworldTiles::TOWER_N);
		--row;
		column += width - 1;
		set2(OverworldTiles::EMPTY);
		--row;
		set2(OverworldTiles::TOWER_S);
		row += 2;
		set2(OverworldTiles::TOWER_N);
		--row;
		column = index % map_width + width / 2;
		for (row = index / map_width - pad; row < index / map_width + height + pad; ++row) {
			buildable_set.erase(row * map_width + column);
			set1(OverworldTiles::ROAD);
		}
		row = index / map_width;
		set2(OverworldTiles::EMPTY);
		--column;
		set2(OverworldTiles::TOWER_NE);
		column += 2;
		set2(OverworldTiles::TOWER_NW);
		--column;
		row += height - 1;
		set2(OverworldTiles::EMPTY);
		--column;
		set2(OverworldTiles::TOWER_NE);
		column += 2;
		set2(OverworldTiles::TOWER_NW);
		--column;

		std::default_random_engine rng;
		rng.seed(666);
		std::vector<Index> buildable(buildable_set.cbegin(), buildable_set.cend());
		std::shuffle(buildable.begin(), buildable.end(), rng);
		Timer timer("Houses");
		if (2 < buildable.size()) {
			buildable.erase(buildable.begin() + buildable.size() / 10, buildable.end());
			buildable_set = std::unordered_set<Index>(buildable.cbegin(), buildable.cend());
			std::vector<TileID> houses {OverworldTiles::HOUSE1, OverworldTiles::HOUSE2, OverworldTiles::HOUSE3};
			while (!buildable_set.empty()) {
				const auto index = *buildable_set.begin();
				const auto house = choose(houses, rng);
				layer2[index] = house;
				const RealmID realm_id = game->newRealmID();
				const Index realm_width = 9;
				const Index realm_height = 9;
				Position house_position {index / map_width, index % map_width};
				auto building = TileEntity::create<Building>(house, house_position, realm_id, realm_width * (realm_height - 1) - 3);
				auto new_tilemap = std::make_shared<Tilemap>(realm_width, realm_height, 16, textureMap.at(Realm::HOUSE));
				auto new_realm = Realm::create(realm_id, Realm::HOUSE, new_tilemap);
				new_realm->game = game;
				new_realm->generateHouse(id, rng, house_position + Position(1, 0), realm_width, realm_height);
				game->realms.emplace(realm_id, new_realm);
				add(building);
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
		timer.stop();
	}

	std::shared_ptr<Entity> Realm::add(const std::shared_ptr<Entity> &entity) {
		entity->setRealm(shared_from_this());
		entities.insert(entity);
		return entity;
	}

	std::shared_ptr<TileEntity> Realm::add(const std::shared_ptr<TileEntity> &tile_entity) {
		const Index index = tile_entity->position.row * tilemap1->width + tile_entity->position.column;
		if (tileEntities.contains(index))
			return nullptr;
		tile_entity->setRealm(shared_from_this());
		tileEntities.emplace(index, tile_entity);
		return tile_entity;
	}

	void Realm::initEntities() {
		for (auto &entity: entities)
			entity->setRealm(shared_from_this());
	}

	void Realm::tick(float delta) {
		for (auto &entity: entities)
			entity->tick(delta);
		for (const auto &entity: removalQueue)
			remove(entity);
		removalQueue.clear();
	}

	std::vector<std::shared_ptr<Entity>> Realm::findEntities(const Position &position) const {
		std::vector<std::shared_ptr<Entity>> out;
		for (const auto &entity: entities)
			if (entity->position == position)
				out.push_back(entity);
		return out;
	}

	std::vector<std::shared_ptr<Entity>> Realm::findEntities(const Position &position, const std::shared_ptr<Entity> &except) const {
		std::vector<std::shared_ptr<Entity>> out;
		for (const auto &entity: entities)
			if (entity->position == position && entity != except)
				out.push_back(entity);
		return out;
	}

	std::shared_ptr<Entity> Realm::findEntity(const Position &position) const {
		for (const auto &entity: entities)
			if (entity->position == position)
				return entity;
		return {};
	}

	std::shared_ptr<Entity> Realm::findEntity(const Position &position, const std::shared_ptr<Entity> &except) const {
		for (const auto &entity: entities)
			if (entity->position == position && entity != except)
				return entity;
		return {};
	}

	std::shared_ptr<TileEntity> Realm::tileEntityAt(const Position &position) const {
		const auto iter = tileEntities.find(position.row * tilemap1->width + position.column);
		if (iter == tileEntities.end())
			return {};
		return iter->second;
	}

	void Realm::remove(const std::shared_ptr<Entity> &entity) {
		entities.erase(entity);
	}

	Position Realm::getPosition(Index index) const {
		return {index / getWidth(), index % getWidth()};
	}

	void Realm::onMoved(const std::shared_ptr<Entity> &entity, const Position &position) {
		if (auto tile_entity = tileEntityAt(position))
			tile_entity->onOverlap(entity);
	}

	void Realm::setGame(Game &game_) {
		game = &game_;
	}

	Game & Realm::getGame() {
		if (!game)
			throw std::runtime_error("Game is null for realm " + std::to_string(id));
		return *game;
	}

	void Realm::queueRemoval(const std::shared_ptr<Entity> &entity) {
		removalQueue.push_back(entity);
	}

	void to_json(nlohmann::json &json, const Realm &realm) {
		json["id"] = realm.id;
		json["type"] = realm.type;
		json["tilemap1"] = *realm.tilemap1;
		json["tilemap2"] = *realm.tilemap2;
		json["tilemap3"] = *realm.tilemap3;
		json["tileEntities"] = std::unordered_map<std::string, nlohmann::json>();
		for (const auto &[index, tile_entity]: realm.tileEntities)
			json["tileEntities"][std::to_string(index)] = *tile_entity;
		json["entities"] = std::vector<nlohmann::json>();
		for (const auto &entity: realm.entities)
			json["entities"].push_back(entity->toJSON());
	}
}
