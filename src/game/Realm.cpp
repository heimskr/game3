#include <iostream>
#include <unordered_set>

#include <libnoise/noise.h>

#include "Tiles.h"
#include "game/Entity.h"
#include "game/Realm.h"
#include "util/Timer.h"
#include "util/Util.h"

namespace Game3 {
	Realm::Realm(RealmID id_, const std::shared_ptr<Tilemap> &tilemap1_, const std::shared_ptr<Tilemap> &tilemap2_, const std::shared_ptr<Tilemap> &tilemap3_):
	id(id_), tilemap1(tilemap1_), tilemap2(tilemap2_), tilemap3(tilemap3_) {
		if (tilemap1)
			renderer1.initialize(tilemap1);
		if (tilemap2)
			renderer2.initialize(tilemap2);
		if (tilemap3)
			renderer3.initialize(tilemap3);
	}

	Realm::Realm(RealmID id_, const std::shared_ptr<Tilemap> &tilemap1_): Realm(id_, tilemap1_, nullptr, nullptr) {
		tilemap2 = std::make_shared<Tilemap>(tilemap1->width, tilemap1->height, tilemap1->tileSize, tilemap1->texture);
		tilemap3 = std::make_shared<Tilemap>(tilemap1->width, tilemap1->height, tilemap1->tileSize, tilemap1->texture);
		renderer2.initialize(tilemap2);
		renderer3.initialize(tilemap3);
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
		const auto width = tilemap1->width;
		const auto height = tilemap1->height;

		noise::module::Perlin perlin;
		perlin.SetSeed(seed);

		auto &tiles1 = tilemap1->tiles;
		tiles1.assign(tiles1.size(), 0);
		tilemap2->tiles.assign(tilemap2->tiles.size(), 0);
		tilemap3->tiles.assign(tilemap3->tiles.size(), 0);

		static const std::vector<TileID> grasses {GRASS_ALT1, GRASS_ALT2, GRASS, GRASS, GRASS, GRASS, GRASS, GRASS, GRASS};

		for (int i = 0; i < width; ++i)
			for (int j = 0; j < height; ++j) {
				double noise = perlin.GetValue(i / noise_zoom, j / noise_zoom, 0.666);
				auto &tile = tiles1[j * width + i];
				if (noise < noise_threshold)
					tile = DEEPER_WATER;
				else if (noise < noise_threshold + 0.1)
					tile = DEEP_WATER;
				else if (noise < noise_threshold + 0.2)
					tile = WATER;
				else if (noise < noise_threshold + 0.3)
					tile = SHALLOW_WATER;
				else if (noise < noise_threshold + 0.4)
					tile = SAND;
				else if (noise < noise_threshold + 0.5)
					tile = LIGHT_GRASS;
				else
					tile = choose(grasses, (i << 20) | j);
			}

		constexpr static int m = 15, n = 21, pad = 2;
		Timer land_timer("GetLand");
		auto starts = tilemap1->getLand(m + pad * 2, n + pad * 2);
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
					if (!isLand(tiles1[row * tilemap1->width + column]))
						goto failed;
			candidates.push_back(index);
			failed:
			continue;
		}
		candidate_timer.stop();
		Timer::summary();

		std::cout << "Found " << candidates.size() << " candidate" << (candidates.size() == 1? "" : "s") << ".\n";
		if (!candidates.empty())
			createTown(choose(candidates, uint_fast32_t(seed)) + pad * (tilemap1->width + 1), n, m, pad);
	}

	void Realm::createTown(const size_t index, size_t width, size_t height, size_t pad) {
		size_t row = 0, column = 0;

		auto &layer1 = tilemap1->tiles;
		auto &layer2 = tilemap2->tiles;

		auto map_width = tilemap1->width;

		auto set1 = [&](TileID tile) { layer1[row * map_width + column] = tile; };
		auto set2 = [&](TileID tile) { layer2[row * map_width + column] = tile; };

		for (size_t row = index / map_width; row < index / map_width + height; ++row) {
			layer2[row * map_width + index % map_width] = TOWER_NS;
			layer2[row * map_width + index % map_width + width - 1] = TOWER_NS;
		}

		for (size_t column = 0; column < width; ++column) {
			layer2[index + column] = TOWER_WE;
			layer2[index + map_width * (height - 1) + column] = TOWER_WE;
		}

		layer2[index] = TOWER_NW;
		layer2[index + map_width * (height - 1)] = TOWER_SW;
		layer2[index + width - 1] = TOWER_NE;
		layer2[index + map_width * (height - 1) + width - 1] = TOWER_SE;

		std::unordered_set<Index> buildable_set;

		for (row = index / map_width + 1; row < index / map_width + height - 1; ++row)
			for (column = index % map_width + 1; column < index % map_width + width - 1; ++column) {
				buildable_set.insert(row * map_width + column);
				set1(DIRT);
			}

		row = index / map_width + height / 2;
		for (column = index % map_width - pad; column < index % map_width + width + pad; ++column) {
			buildable_set.erase(row * map_width + column);
			set1(ROAD);
		}
		column = index % map_width;
		set2(EMPTY);
		--row;
		set2(TOWER_S);
		row += 2;
		set2(TOWER_N);
		--row;
		column += width - 1;
		set2(EMPTY);
		--row;
		set2(TOWER_S);
		row += 2;
		set2(TOWER_N);
		--row;
		column = index % map_width + width / 2;
		for (row = index / map_width - pad; row < index / map_width + height + pad; ++row) {
			buildable_set.erase(row * map_width + column);
			set1(ROAD);
		}
		row = index / map_width;
		set2(EMPTY);
		--column;
		set2(TOWER_NE);
		column += 2;
		set2(TOWER_NW);
		--column;
		row += height - 1;
		set2(EMPTY);
		--column;
		set2(TOWER_NE);
		column += 2;
		set2(TOWER_NW);
		--column;

		std::vector<Index> buildable(buildable_set.cbegin(), buildable_set.cend());
		shuffle(buildable, 666);
		if (2 < buildable.size()) {
			buildable.erase(buildable.begin() + buildable.size() / 10, buildable.end());
			buildable_set = std::unordered_set<Index>(buildable.cbegin(), buildable.cend());
			std::vector<TileID> houses {HOUSE1, HOUSE2, HOUSE3};
			std::default_random_engine rng;
			rng.seed(666);
			while (!buildable_set.empty()) {
				auto index = *buildable_set.begin();
				auto house = choose(houses, rng);
				layer2[index] = house;
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
	}

	void Realm::addEntity(const std::shared_ptr<Entity> &entity) {
		entity->setRealm(shared_from_this());
		entities.insert(entity);
	}

	void Realm::initEntities() {
		for (auto &entity: entities)
			entity->setRealm(shared_from_this());
	}

	void to_json(nlohmann::json &json, const Realm &realm) {
		json["id"] = realm.id;
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

	void from_json(const nlohmann::json &json, Realm &realm) {
		realm.id = json.at("id");
		realm.tilemap1 = std::make_shared<Tilemap>(json.at("tilemap1"));
		realm.tilemap2 = std::make_shared<Tilemap>(json.at("tilemap2"));
		realm.tilemap3 = std::make_shared<Tilemap>(json.at("tilemap3"));
		realm.tilemap1->texture.init();
		realm.tilemap2->texture.init();
		realm.tilemap3->texture.init();
		for (const auto &[index, tile_entity_json]: json.at("tileEntities").get<std::unordered_map<std::string, nlohmann::json>>())
			realm.tileEntities.emplace(parseUlong(index), TileEntity::fromJSON(tile_entity_json));
		realm.renderer1.initialize(realm.tilemap1);
		realm.renderer2.initialize(realm.tilemap2);
		realm.renderer3.initialize(realm.tilemap3);
		realm.entities.clear();
		for (const auto &entity_json: json.at("entities"))
			realm.entities.insert(Entity::fromJSON(entity_json));
	}
}
