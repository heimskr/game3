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
#include "tileentity/Town.h"
#include "util/Timer.h"
#include "util/Util.h"

namespace Game3 {
	std::unordered_map<RealmType, Texture> Realm::textureMap {
		{Realm::OVERWORLD, Texture("resources/tileset2.png")},
		{Realm::HOUSE,     Texture("resources/house/house.png")},
	};

	Realm::Realm(RealmID id_, RealmType type_, const std::shared_ptr<Tilemap> &tilemap1_, const std::shared_ptr<Tilemap> &tilemap2_, const std::shared_ptr<Tilemap> &tilemap3_):
	id(id_), type(type_), tilemap1(tilemap1_), tilemap2(tilemap2_), tilemap3(tilemap3_) {
		tilemap1->init();
		tilemap2->init();
		tilemap3->init();
		renderer1.init(tilemap1);
		renderer2.init(tilemap2);
		renderer3.init(tilemap3);
		resetPathMap();
	}

	Realm::Realm(RealmID id_, RealmType type_, const std::shared_ptr<Tilemap> &tilemap1_): id(id_), type(type_), tilemap1(tilemap1_) {
		tilemap1->init();
		renderer1.init(tilemap1);
		tilemap2 = std::make_shared<Tilemap>(tilemap1->width, tilemap1->height, tilemap1->tileSize, tilemap1->texture);
		tilemap3 = std::make_shared<Tilemap>(tilemap1->width, tilemap1->height, tilemap1->tileSize, tilemap1->texture);
		tilemap2->init();
		tilemap3->init();
		renderer2.init(tilemap2);
		renderer3.init(tilemap3);
		resetPathMap();
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
		if (json.contains("extra"))
			realm.extraData = json.at("extra");
		return out;
	}

	void Realm::render(const int width, const int height, const Eigen::Vector2f &center, float scale, SpriteRenderer &sprite_renderer, float game_time) {
		renderer1.center = center;
		renderer1.scale  = scale;
		renderer1.onBackbufferResized(width, height);
		renderer2.center = center;
		renderer2.scale  = scale;
		renderer2.onBackbufferResized(width, height);
		renderer3.center = center;
		renderer3.scale  = scale;
		renderer3.onBackbufferResized(width, height);
		renderer1.render(game_time);
		renderer2.render(game_time);
		renderer3.render(game_time);
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
		auto &tiles2 = tilemap2->tiles;
		tiles1.assign(tiles1.size(), 0);
		tiles2.assign(tiles2.size(), 0);
		tilemap3->tiles.assign(tilemap3->tiles.size(), 0);

		static const std::vector<TileID> grasses {
			OverworldTiles::GRASS_ALT1, OverworldTiles::GRASS_ALT2,
			OverworldTiles::GRASS, OverworldTiles::GRASS, OverworldTiles::GRASS, OverworldTiles::GRASS, OverworldTiles::GRASS, OverworldTiles::GRASS, OverworldTiles::GRASS
		};

		std::default_random_engine rng;
		rng.seed(seed);

		auto saved_noise = std::make_unique<double[]>(width * height);

		Timer noise_timer("Noise");
		for (int row = 0; row < height; ++row)
			for (int column = 0; column < width; ++column) {
				double noise = perlin.GetValue(row / noise_zoom, column / noise_zoom, 0.666);
				saved_noise[row * width + column] = noise;
				if (noise < noise_threshold)
					setLayer1(row, column, OverworldTiles::DEEPER_WATER);
				else if (noise < noise_threshold + 0.1)
					setLayer1(row, column, OverworldTiles::DEEP_WATER);
				else if (noise < noise_threshold + 0.2)
					setLayer1(row, column, OverworldTiles::WATER);
				else if (noise < noise_threshold + 0.3)
					setLayer1(row, column, OverworldTiles::SHALLOW_WATER);
				else if (noise < noise_threshold + 0.4)
					setLayer1(row, column, OverworldTiles::SAND);
				else if (noise < noise_threshold + 0.5)
					setLayer1(row, column, OverworldTiles::LIGHT_GRASS);
				else
					setLayer1(row, column, choose(grasses, rng));
			}
		noise_timer.stop();

		constexpr static int m = 26, n = 34, pad = 2;
		Timer land_timer("GetLand");
		auto starts = tilemap1->getLand(type, m + pad * 2, n + pad * 2);
		if (starts.empty())
			throw std::runtime_error("Map has no land");
		land_timer.stop();

		Timer oil_timer("Oil");
		auto oil_starts = tilemap1->getLand(type);
		std::shuffle(oil_starts.begin(), oil_starts.end(), rng);
		for (size_t i = 0, max = oil_starts.size() / 2000; i < max; ++i) {
			const Index index = oil_starts.back();
			if (noise_threshold + 0.6 <= saved_noise[index])
				setLayer2(index, OverworldTiles::OIL);
			oil_starts.pop_back();
		}
		oil_timer.stop();

		randomLand = choose(starts, rng);
		std::vector<Index> candidates;
		candidates.reserve(starts.size() / 16);
		Timer candidate_timer("Candidates");
		for (const auto index: starts) {
			const size_t row_start = index / tilemap1->width + pad, row_end = row_start + m;
			const size_t column_start = index % tilemap1->width + pad, column_end = column_start + n;
			for (size_t row = row_start; row < row_end; ++row)
				for (size_t column = column_start; column < column_end; ++column) {
					const Index index = row * tilemap1->width + column;
					if (!overworldTiles.isLand(tiles1[index]))
						goto failed;
				}
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
		for (int column = 1; column < width - 1; ++column) {
			setLayer2(column, HouseTiles::WALL_WEN);
			setLayer2(height - 1, column, HouseTiles::WALL_WES);
		}

		for (int row = 1; row < height - 1; ++row) {
			setLayer2(row, 0, HouseTiles::WALL_NS);
			setLayer2(row, width - 1, HouseTiles::WALL_NS);
		}

		for (int row = 0; row < height; ++row)
			for (int column = 0; column < width; ++column)
				setLayer1(row, column, HouseTiles::FLOOR);

		setLayer2(0, HouseTiles::WALL_NW);
		setLayer2(width - 1, HouseTiles::WALL_NE);
		setLayer2(width * (height - 1), HouseTiles::WALL_SW);
		setLayer2(width * height - 1, HouseTiles::WALL_SE);

		const Index exit_index = width * height - 3;
		setLayer2(exit_index - 1, HouseTiles::WALL_W);
		setLayer2(exit_index,     HouseTiles::EMPTY);
		setLayer2(exit_index + 1, HouseTiles::WALL_E);

		static std::array<TileID, 3> plants {HouseTiles::PLANT1, HouseTiles::PLANT2, HouseTiles::PLANT3};
		static std::array<TileID, 3> beds   {HouseTiles::BED1,   HouseTiles::BED2,   HouseTiles::BED3};
		static std::array<TileID, 2> doors  {HouseTiles::DOOR1,  HouseTiles::DOOR2};

		setLayer2(width + 1, choose(plants, rng));
		setLayer2(2 * width - 2, choose(plants, rng));
		setLayer2((width - 1) * height - 2, choose(plants, rng));
		setLayer2((width - 2) * height + 1, choose(plants, rng));

		std::array<Index, 2> edges {1, width - 2};
		const Position bed_position(2 + rng() % (height - 4), choose(edges, rng));
		setLayer2(getIndex(bed_position), choose(beds, rng));
		extraData["bed"] = bed_position;

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
					setLayer2(index, HouseTiles::BOOKSHELF);
					add(TileEntity::create<Sign>(HouseTiles::EMPTY, getPosition(index), shuffled_texts.at((index - width - 2) % shuffled_texts.size()), "Bookshelf"));
				}
				break;
			}

			case 1: {
				auto chest = TileEntity::create<Chest>(0, getPosition(width * 3 / 2), "Chest");
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
				setLayer1(row * width + column, HouseTiles::CARPET_C + carpet_offset);
		for (int row = carpet_padding + 1; row < height - carpet_padding - 1; ++row) {
			setLayer1(row * width + carpet_padding, HouseTiles::CARPET_W + carpet_offset);
			setLayer1((row + 1) * width - carpet_padding - 1, HouseTiles::CARPET_E + carpet_offset);
		}
		for (int column = carpet_padding + 1; column < width - carpet_padding - 1; ++column) {
			setLayer1(carpet_padding * width + column, HouseTiles::CARPET_N + carpet_offset);
			setLayer1((height - carpet_padding - 1) * width + column, HouseTiles::CARPET_S + carpet_offset);
		}
		setLayer1(carpet_padding * width + carpet_padding, HouseTiles::CARPET_NW + carpet_offset);
		setLayer1((carpet_padding + 1) * width - carpet_padding - 1, HouseTiles::CARPET_NE + carpet_offset);
		setLayer1((height - carpet_padding - 1) * width + carpet_padding, HouseTiles::CARPET_SW + carpet_offset);
		setLayer1((height - carpet_padding) * width - carpet_padding - 1, HouseTiles::CARPET_SE + carpet_offset);
	}

	void Realm::createTown(const size_t index, size_t width, size_t height, size_t pad) {
		size_t row = 0, column = 0;

		auto map_width = tilemap1->width;

		auto set1 = [&](TileID tile) { setLayer1(row, column, tile); };
		auto set2 = [&](TileID tile) { setLayer2(row, column, tile); };

		for (size_t row = index / map_width - pad; row < index / map_width + height + pad; ++row)
			for (size_t column = index % map_width - pad; column < index % map_width + width + pad; ++column)
				setLayer2(row * map_width + column, OverworldTiles::EMPTY);

		for (size_t row = index / map_width; row < index / map_width + height; ++row) {
			setLayer2(row * map_width + index % map_width, OverworldTiles::TOWER_NS);
			setLayer2(row * map_width + index % map_width + width - 1, OverworldTiles::TOWER_NS);
		}

		for (size_t column = 0; column < width; ++column) {
			setLayer2(index + column, OverworldTiles::TOWER_WE);
			setLayer2(index + map_width * (height - 1) + column, OverworldTiles::TOWER_WE);
		}

		setLayer2(index, OverworldTiles::TOWER_NW);
		setLayer2(index + map_width * (height - 1), OverworldTiles::TOWER_SW);
		setLayer2(index + width - 1, OverworldTiles::TOWER_NE);
		setLayer2(index + map_width * (height - 1) + width - 1, OverworldTiles::TOWER_SE);

		std::unordered_set<Index> buildable_set;

		for (row = index / map_width + 1; row < index / map_width + height - 1; ++row)
			for (column = index % map_width + 1; column < index % map_width + width - 1; ++column) {
				buildable_set.insert(row * map_width + column);
				set1(OverworldTiles::DIRT);
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

		// setLayer2(index / map_width + height / 2, index % map_width + width / 2, 

		std::default_random_engine rng;
		rng.seed(666);
		std::vector<Index> buildable(buildable_set.cbegin(), buildable_set.cend());
		std::shuffle(buildable.begin(), buildable.end(), rng);
		Timer timer("Houses");
		if (2 < buildable.size()) {
			buildable.erase(buildable.begin() + buildable.size() / 10, buildable.end());
			buildable_set = std::unordered_set<Index>(buildable.cbegin(), buildable.cend());
			while (!buildable_set.empty()) {
				const auto index = *buildable_set.begin();
				if (rng() % 8 == 0) {
					constexpr static std::array<TileID, 3> markets {OverworldTiles::MARKET1, OverworldTiles::MARKET2, OverworldTiles::MARKET3};
					const auto market = choose(markets, rng);
					setLayer2(index, market);
				} else {
					constexpr static std::array<TileID, 3> houses {OverworldTiles::HOUSE1, OverworldTiles::HOUSE2, OverworldTiles::HOUSE3};
					const auto house = choose(houses, rng);
					setLayer2(index, house);
					const RealmID realm_id = game->newRealmID();
					const Index realm_width = 9;
					const Index realm_height = 9;
					Position house_position {index / map_width, index % map_width};
					auto building = TileEntity::create<Building>(house, house_position, realm_id, realm_width * (realm_height - 1) - 3);
					auto new_tilemap = std::make_shared<Tilemap>(realm_width, realm_height, 16, textureMap.at(Realm::HOUSE));
					auto new_realm = Realm::create(realm_id, Realm::HOUSE, new_tilemap);
					new_realm->game = game;
					new_realm->generateHouse(id, rng, house_position + Position {1, 0}, realm_width, realm_height);
					game->realms.emplace(realm_id, new_realm);
					add(building);
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
		ticking = true;
		for (auto &entity: entities)
			if (entity->isPlayer()) {
				auto player = std::dynamic_pointer_cast<Player>(entity);
				if (!player->ticked) {
					player->ticked = true;
					player->tick(delta);
				}
			} else
				entity->tick(delta);
		ticking = false;
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
		if (ticking)
			removalQueue.push_back(entity);
		else
			remove(entity);
	}

	void Realm::absorb(const std::shared_ptr<Entity> &entity, const Position &position) {
		if (auto realm = entity->weakRealm.lock())
			realm->remove(entity);
		entity->setRealm(shared_from_this());
		entity->init();
		entity->teleport(position);
	}

	void Realm::setLayer1(Index row, Index column, TileID tile) {
		(*tilemap1)(column, row) = tile;
		setLayerHelper(row, column);
	}

	void Realm::setLayer2(Index row, Index column, TileID tile) {
		(*tilemap2)(column, row) = tile;
		setLayerHelper(row, column);
	}

	void Realm::setLayer3(Index row, Index column, TileID tile) {
		(*tilemap3)(column, row) = tile;
		setLayerHelper(row, column);
	}

	void Realm::setLayer1(Index index, TileID tile) {
		tilemap1->tiles[index] = tile;
		setLayerHelper(index);
	}

	void Realm::setLayer2(Index index, TileID tile) {
		tilemap2->tiles[index] = tile;
		setLayerHelper(index);
	}

	void Realm::setLayer3(Index index, TileID tile) {
		tilemap3->tiles[index] = tile;
		setLayerHelper(index);
	}

	void Realm::setLayerHelper(Index row, Index column) {
		const auto &tileset = tileSets.at(type);
		pathMap[getIndex(row, column)] = tileset->isWalkable((*tilemap1)(column, row)) && tileset->isWalkable((*tilemap2)(column, row)) && tileset->isWalkable((*tilemap3)(column, row));
	}

	void Realm::setLayerHelper(Index index) {
		const auto &tileset = tileSets.at(type);
		const auto row    = index / getWidth();
		const auto column = index % getWidth();
		pathMap[index] = tileset->isWalkable((*tilemap1)(column, row)) && tileset->isWalkable((*tilemap2)(column, row)) && tileset->isWalkable((*tilemap3)(column, row));
	}

	void Realm::resetPathMap() {
		const auto width = tilemap1->width;
		const auto height = tilemap1->height;
		pathMap.resize(width * height);
		const auto &tileset = tileSets.at(type);
		for (Index row = 0; row < height; ++row)
			for (Index column = 0; column < width; ++column)
				pathMap[getIndex(row, column)] = tileset->isWalkable((*tilemap1)(column, row)) && tileset->isWalkable((*tilemap2)(column, row)) && tileset->isWalkable((*tilemap3)(column, row));
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
		if (!realm.extraData.empty())
			json["extra"] = realm.extraData;
	}
}
