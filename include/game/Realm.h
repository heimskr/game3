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
	class Game;
	class SpriteRenderer;

	class Realm: public std::enable_shared_from_this<Realm> {
		public:
			constexpr static RealmType OVERWORLD = 1;
			constexpr static RealmType HOUSE = 2;

			static std::unordered_map<RealmType, Texture> textureMap;

			Game *game = nullptr;
			RealmID id;
			RealmType type;
			std::shared_ptr<Tilemap> tilemap1, tilemap2, tilemap3;
			ElementBufferedRenderer renderer1, renderer2, renderer3;
			std::unordered_map<Index, std::shared_ptr<TileEntity>> tileEntities;
			std::unordered_set<std::shared_ptr<Entity>> entities;

			Realm(const Realm &) = delete;
			Realm(Realm &&) = default;

			Realm & operator=(const Realm &) = delete;
			Realm & operator=(Realm &&) = default;

			template <typename... Args>
			static std::shared_ptr<Realm> create(Args && ...args) {
				auto out = std::shared_ptr<Realm>(new Realm(std::forward<Args>(args)...));
				return out;
			}

			static std::shared_ptr<Realm> fromJSON(const nlohmann::json &);

			void render(int width, int height, const nanogui::Vector2f &center, float scale, SpriteRenderer &);
			void reupload();
			void rebind();
			void generate(int seed = 666, double noise_zoom = 100., double noise_threshold = -0.15);
			void generateHouse(RealmID parent_realm, const Position &entrance, int width, int height);
			void createTown(size_t index, size_t width, size_t height, size_t pad);
			int getWidth()  const { return tilemap1->width;  }
			int getHeight() const { return tilemap1->height; }
			std::shared_ptr<Entity> add(const std::shared_ptr<Entity> &);
			std::shared_ptr<TileEntity> add(const std::shared_ptr<TileEntity> &);
			void initEntities();
			void tick(float delta);
			std::vector<std::shared_ptr<Entity>> findEntities(const Position &) const;
			std::vector<std::shared_ptr<Entity>> findEntities(const Position &, const std::shared_ptr<Entity> &except) const;
			std::shared_ptr<Entity> findEntity(const Position &) const;
			std::shared_ptr<Entity> findEntity(const Position &, const std::shared_ptr<Entity> &except) const;
			std::shared_ptr<TileEntity> tileEntityAt(const Position &) const;
			void remove(const std::shared_ptr<Entity> &);
			Position getPosition(Index) const;
			void onMoved(const std::shared_ptr<Entity> &, const Position &);

			template <typename T, typename... Args>
			std::shared_ptr<T> spawn(const Position &position, Args && ...args) {
				auto entity = T::create(std::forward<Args>(args)...);
				entity->setRealm(shared_from_this());
				entity->init();
				entity->teleport(position);
				add(entity);
				return entity;
			}

			friend class MainWindow;

		protected:
			Realm() = default;
			Realm(RealmID id_, RealmType type_, const std::shared_ptr<Tilemap> &tilemap1_, const std::shared_ptr<Tilemap> &tilemap2_, const std::shared_ptr<Tilemap> &tilemap3_);
			Realm(RealmID id_, RealmType type_, const std::shared_ptr<Tilemap> &tilemap1_);

		private:
			Index randomLand = 0;
	};

	void to_json(nlohmann::json &, const Realm &);
}
