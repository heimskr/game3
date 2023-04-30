#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <unordered_map>
#include <unordered_set>

#include <nlohmann/json.hpp>

#include "Tilemap.h"
#include "Types.h"
#include "game/BiomeMap.h"
#include "game/TileProvider.h"
#include "tileentity/TileEntity.h"
#include "ui/ElementBufferedRenderer.h"
#include "util/GL.h"
#include "util/RWLock.h"

namespace Game3 {
	constexpr size_t REALM_DIAMETER = 3;

	class Entity;
	class Game;
	class SpriteRenderer;

	using EntityPtr = std::shared_ptr<Entity>;

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
			RealmID id;
			RealmType type;
			TileProvider tileProvider;
			BiomeMapPtr biomeMap;
			std::array<std::array<std::array<ElementBufferedRenderer, LAYER_COUNT>, REALM_DIAMETER>, REALM_DIAMETER> renderers {};
			std::unordered_map<Position, std::shared_ptr<TileEntity>> tileEntities;
			std::unordered_set<EntityPtr> entities;
			nlohmann::json extraData;
			Position randomLand;
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
			/** The Layer argument is 1-based. */
			void reupload(Layer);
			void rebind();
			EntityPtr add(const EntityPtr &);
			std::shared_ptr<TileEntity> add(const std::shared_ptr<TileEntity> &);
			std::shared_ptr<TileEntity> addUnsafe(const std::shared_ptr<TileEntity> &);
			void initEntities();
			void tick(float delta);
			std::vector<EntityPtr> findEntities(const Position &) const;
			std::vector<EntityPtr> findEntities(const Position &, const EntityPtr &except) const;
			EntityPtr findEntity(const Position &) const;
			EntityPtr findEntity(const Position &, const EntityPtr &except) const;
			std::shared_ptr<TileEntity> tileEntityAt(const Position &);
			void remove(EntityPtr);
			void remove(const std::shared_ptr<TileEntity> &, bool run_helper = true);
			void removeSafe(const std::shared_ptr<TileEntity> &);
			void onMoved(const EntityPtr &, const Position &);
			Game & getGame();
			void queueRemoval(const EntityPtr &);
			void queueRemoval(const std::shared_ptr<TileEntity> &);
			void absorb(const EntityPtr &, const Position &);
			void setTile(Layer, Index row, Index column, TileID, bool run_helper = true);
			void setTile(Layer, const Position &, TileID, bool run_helper = true);
			void setTile(Layer, const Position &, const Identifier &, bool run_helper = true);
			TileID getTile(Layer, Index row, Index column) const;
			TileID getTile(Layer, const Position &) const;
			std::optional<Position> getPathableAdjacent(const Position &) const;
			bool hasTileEntityAt(const Position &) const;
			void confirmGhosts();
			void damageGround(const Position &);
			Tileset & getTileset();
			/** Redoes the pathmap for the entire stored map, not just the visible ones! Can be very expensive. */
			void remakePathMap();

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
			Realm(Game &, RealmID, RealmType, int seed_);

			void initTexture();
			virtual void absorbJSON(const nlohmann::json &);
			virtual void toJSON(nlohmann::json &) const;

		private:
			Game &game;
			bool ticking = false;
			std::vector<EntityPtr> entityRemovalQueue;
			std::vector<std::shared_ptr<TileEntity>> tileEntityRemovalQueue;
			RWLock tileEntityLock;

			void initRendererRealms();
			void initRendererTileProviders();

			bool isWalkable(Index row, Index column, const Tileset &) const;
			void setLayerHelper(Index row, Index col, bool should_mark_dirty = true);

			static BiomeType getBiome(uint32_t seed);
	};

	void to_json(nlohmann::json &, const Realm &);

	using RealmPtr = std::shared_ptr<Realm>;
}
