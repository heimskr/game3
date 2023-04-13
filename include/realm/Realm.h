#pragma once

#include <memory>
#include <optional>
#include <random>
#include <unordered_map>
#include <unordered_set>

#include <nlohmann/json.hpp>

#include "Tilemap.h"
#include "Types.h"
#include "game/BiomeMap.h"
#include "tileentity/TileEntity.h"
#include "ui/ElementBufferedRenderer.h"
#include "util/GL.h"

namespace Game3 {
	class Entity;
	class Game;
	class SpriteRenderer;

	struct RealmDetails: NamedRegisterable {
		Identifier tilesetName;
		RealmDetails():
			NamedRegisterable(Identifier()) {}
		RealmDetails(Identifier identifier_, Identifier tileset_name):
			NamedRegisterable(std::move(identifier_)),
			tilesetName(std::move(tileset_name)) {}
	};

	void from_json(const nlohmann::json &, RealmDetails &);

	class Realm: public std::enable_shared_from_this<Realm> {
		public:
			GL::Texture texture;
			GL::Texture otherTexture;
			GL::FBO fbo;
			RealmID id;
			RealmType type;
			TilemapPtr tilemap1;
			TilemapPtr tilemap2;
			TilemapPtr tilemap3;
			BiomeMapPtr biomeMap;
			ElementBufferedRenderer renderer1 {*this};
			ElementBufferedRenderer renderer2 {*this};
			ElementBufferedRenderer renderer3 {*this};
			std::unordered_map<Index, std::shared_ptr<TileEntity>> tileEntities;
			std::unordered_set<std::shared_ptr<Entity>> entities;
			/** A vector of bools (represented with uint8_t to avoid the std::vector<bool> specialization) indicating whether a given square is empty for the purposes of pathfinding. */
			std::vector<uint8_t> pathMap;
			nlohmann::json extraData;
			Index randomLand = 0;
			/** Whether the realm's rendering should be affected by the day-night cycle. */
			bool outdoors = true;
			size_t ghostCount = 0;
			uint32_t seed = 0;

			Realm(const Realm &) = delete;
			Realm(Realm &&) = delete;

			virtual ~Realm() = default;

			Realm & operator=(const Realm &) = delete;
			Realm & operator=(Realm &&) = delete;

			template <typename T = Realm, typename... Args>
			static std::shared_ptr<T> create(Args && ...args) {
				return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
			}

			static std::shared_ptr<Realm> fromJSON(Game &, const nlohmann::json &);

			void render(int width, int height, const Eigen::Vector2f &center, float scale, SpriteRenderer &, float game_time);
			void reupload();
			void rebind();
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
			void remove(std::shared_ptr<Entity>);
			void remove(std::shared_ptr<TileEntity>);
			Position getPosition(Index) const;
			void onMoved(const std::shared_ptr<Entity> &, const Position &);
			Game & getGame();
			void queueRemoval(const std::shared_ptr<Entity> &);
			void queueRemoval(const std::shared_ptr<TileEntity> &);
			void absorb(const std::shared_ptr<Entity> &, const Position &);
			void setLayer1(Index row, Index column, TileID);
			void setLayer2(Index row, Index column, TileID);
			void setLayer3(Index row, Index column, TileID);
			void setLayer1(Index, TileID);
			void setLayer2(Index, TileID);
			void setLayer3(Index, TileID);
			void setLayer1(Index, const Identifier &);
			void setLayer2(Index, const Identifier &);
			void setLayer3(Index, const Identifier &);
			void setLayer1(const Position &, TileID);
			void setLayer2(const Position &, TileID);
			void setLayer3(const Position &, TileID);
			void setLayer1(const Position &, const Identifier &);
			void setLayer2(const Position &, const Identifier &);
			void setLayer3(const Position &, const Identifier &);
			TileID getLayer1(Index row, Index column) const;
			TileID getLayer2(Index row, Index column) const;
			TileID getLayer3(Index row, Index column) const;
			TileID getLayer1(Index) const;
			TileID getLayer2(Index) const;
			TileID getLayer3(Index) const;
			TileID getLayer1(const Position &) const;
			TileID getLayer2(const Position &) const;
			TileID getLayer3(const Position &) const;
			inline Index getIndex(const Position &position) const { return position.row * getWidth() + position.column; }
			inline Index getIndex(Index row, Index column) const { return row * getWidth() + column; }
			std::optional<Position> getPathableAdjacent(const Position &) const;
			std::optional<Position> getPathableAdjacent(Index) const;
			bool isValid(const Position &) const;
			bool hasTileEntityAt(const Position &) const;
			void confirmGhosts();
			void damageGround(const Position &);
			const Tileset & getTileset() const;

			virtual bool interactGround(const std::shared_ptr<Player> &, const Position &);
			virtual void updateNeighbors(const Position &);
			/** Returns true iff something was done with the right click. */
			virtual bool rightClick(const Position &, double x, double y);

			template <typename T, typename... Args>
			std::shared_ptr<T> spawn(const Position &position, Args && ...args) {
				Game &game_ref = getGame();
				auto entity = T::create(game_ref, std::forward<Args>(args)...);
				entity->setRealm(shared_from_this());
				entity->init(game_ref);
				entity->teleport(position);
				add(entity);
				return entity;
			}

			template <typename T>
			std::shared_ptr<T> getTileEntity() const {
				std::shared_ptr<T> out;
				for (const auto &[index, tile_entity]: tileEntities)
					if (auto cast = std::dynamic_pointer_cast<T>(tile_entity)) {
						if (out)
							throw std::runtime_error("Multiple tile entities of type " + std::string(typeid(T).name()) + " found");
						out = cast;
					}
				if (!out)
					throw std::runtime_error("No tile entities of type " + std::string(typeid(T).name()) + " found");
				return out;
			}

			template <typename T, typename P>
			std::shared_ptr<T> getTileEntity(const P &predicate) const {
				std::shared_ptr<T> out;
				for (const auto &[index, tile_entity]: tileEntities)
					if (auto cast = std::dynamic_pointer_cast<T>(tile_entity)) {
						if (predicate(cast)) {
							if (out)
								throw std::runtime_error("Multiple tile entities of type " + std::string(typeid(T).name()) + " found");
							out = cast;
						}
					}
				if (!out)
					throw std::runtime_error("No tile entities of type " + std::string(typeid(T).name()) + " found");
				return out;
			}

			template <typename T>
			std::shared_ptr<T> closestTileEntity(const Position &position) const {
				double minimum_distance = INFINITY;
				std::shared_ptr<T> out;
				for (const auto &[index, entity]: tileEntities)
					if (auto cast = std::dynamic_pointer_cast<T>(entity)) {
						const double distance = entity->position.distance(position);
						if (distance < minimum_distance) {
							minimum_distance = distance;
							out = cast;
						}
					}
				if (!out)
					throw std::runtime_error("No tile entities of type " + std::string(typeid(T).name()) + " found");
				return out;
			}

			template <typename T, typename P>
			std::shared_ptr<T> closestTileEntity(const Position &position, const P &predicate) const {
				double minimum_distance = INFINITY;
				std::shared_ptr<T> out;
				for (const auto &[index, entity]: tileEntities)
					if (auto cast = std::dynamic_pointer_cast<T>(entity)) {
						const double distance = entity->position.distance(position);
						if (predicate(cast) && distance < minimum_distance) {
							minimum_distance = distance;
							out = cast;
						}
					}
				if (!out)
					throw std::runtime_error("No tile entities of type " + std::string(typeid(T).name()) + " found");
				return out;
			}

			friend class MainWindow;
			friend void to_json(nlohmann::json &, const Realm &);

		protected:
			Realm(Game &);
			Realm(Game &, RealmID, RealmType, TilemapPtr tilemap1_, TilemapPtr tilemap2_, TilemapPtr tilemap3_, BiomeMapPtr, int seed_);
			Realm(Game &, RealmID, RealmType, TilemapPtr tilemap1_, BiomeMapPtr, int seed_);

			void initTexture();
			virtual void absorbJSON(const nlohmann::json &);
			virtual void toJSON(nlohmann::json &) const;

		private:
			Game &game;
			bool ticking = false;
			std::vector<std::shared_ptr<Entity>> entityRemovalQueue;
			std::vector<std::shared_ptr<TileEntity>> tileEntityRemovalQueue;

			bool isWalkable(Index row, Index column, const Tileset &) const;
			void setLayerHelper(Index row, Index col, bool should_mark_dirty = true);
			void setLayerHelper(Index, bool should_mark_dirty = true);
			void resetPathMap();

			static BiomeType getBiome(uint32_t seed);
	};

	void to_json(nlohmann::json &, const Realm &);

	using RealmPtr = std::shared_ptr<Realm>;
}
