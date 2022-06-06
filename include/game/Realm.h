#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <nlohmann/json.hpp>

#include "Tilemap.h"
#include "Types.h"
#include "game/TileEntity.h"
#include "ui/ElementBufferedRenderer.h"

namespace Game3 {
	class Entity;
	class SpriteRenderer;

	class Realm: public std::enable_shared_from_this<Realm> {
		public:
			RealmID id;
			std::shared_ptr<Tilemap> tilemap1, tilemap2, tilemap3;
			ElementBufferedRenderer renderer1, renderer2, renderer3;
			std::unordered_map<Index, std::shared_ptr<TileEntity>> tileEntities;
			std::unordered_set<std::shared_ptr<Entity>> entities;

			Realm() = default;
			Realm(RealmID id_, const std::shared_ptr<Tilemap> &tilemap1_, const std::shared_ptr<Tilemap> &tilemap2_, const std::shared_ptr<Tilemap> &tilemap3_);
			Realm(RealmID id_, const std::shared_ptr<Tilemap> &tilemap1_);

			void render(int width, int height, const nanogui::Vector2f &center, float scale, SpriteRenderer &);
			void reupload();
			void rebind();
			void generate(int seed = 666, double noise_zoom = 100., double noise_threshold = -0.15);
			void createTown(size_t index, size_t width, size_t height, size_t pad);
			int getWidth()  const { return tilemap1->width;  }
			int getHeight() const { return tilemap1->height; }
			std::shared_ptr<Entity> addEntity(const std::shared_ptr<Entity> &);
			void initEntities();
			void tick(float delta);

			friend class MainWindow;

		private:
			Index randomLand = 0;
	};

	void to_json(nlohmann::json &, const Realm &);
	void from_json(const nlohmann::json &, Realm &);
}
