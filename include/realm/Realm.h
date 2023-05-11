#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <set>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include <nlohmann/json.hpp>

#include "Types.h"
#include "game/BiomeMap.h"
#include "game/TileProvider.h"
#include "tileentity/TileEntity.h"
#include "ui/ElementBufferedRenderer.h"
#include "ui/Modifiers.h"
#include "util/GL.h"
#include "util/MTQueue.h"
#include "util/RWLock.h"
#include "util/SharedRecursiveMutex.h"

namespace Game3 {
	constexpr int64_t REALM_DIAMETER = 3;

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
		private:
			struct Pauser {
				std::shared_ptr<Realm> realm;
				Pauser(std::shared_ptr<Realm> realm_): realm(realm_) { realm->updatesPaused = true; }
				~Pauser() { realm->updatesPaused = false; }
			};

		public:
			RealmID id;
			RealmType type;
			TileProvider tileProvider;
			std::array<std::array<std::array<ElementBufferedRenderer, LAYER_COUNT>, REALM_DIAMETER>, REALM_DIAMETER> renderers {};
			/** Governed by tileEntityMutex. */
			std::unordered_map<Position, std::shared_ptr<TileEntity>> tileEntities;
			/** Governed by tileEntityMutex. */
			std::unordered_map<GlobalID, std::shared_ptr<TileEntity>> tileEntitiesByGID;
			/** Governed by entityMutex. */
			std::unordered_set<EntityPtr> entities;
			/** Governed by entityMutex. */
			std::unordered_set<PlayerPtr> players;
			nlohmann::json extraData;
			Position randomLand;
			/** Whether the realm's rendering should be affected by the day-night cycle. */
			bool outdoors = true;
			size_t ghostCount = 0;
			int seed = 0;
			std::set<ChunkPosition> generatedChunks;

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

			virtual void onFocus();
			virtual void onBlur();
			void render(int width, int height, const Eigen::Vector2f &center, float scale, SpriteRenderer &, float game_time);
			void reupload();
			/** The Layer argument is 1-based. */
			void reupload(Layer);
			EntityPtr addUnsafe(const EntityPtr &);
			EntityPtr add(const EntityPtr &);
			std::shared_ptr<TileEntity> add(const TileEntityPtr &);
			std::shared_ptr<TileEntity> addUnsafe(const TileEntityPtr &);
			void initEntities();
			void tick(float delta);
			std::vector<EntityPtr> findEntities(const Position &);
			std::vector<EntityPtr> findEntities(const Position &, const EntityPtr &except);
			EntityPtr findEntity(const Position &);
			EntityPtr findEntity(const Position &, const EntityPtr &except);
			std::shared_ptr<TileEntity> tileEntityAt(const Position &);
			void remove(const EntityPtr &);
			void removeSafe(const EntityPtr &);
			void remove(TileEntityPtr, bool run_helper = true);
			void removeSafe(const TileEntityPtr &);
			void onMoved(const EntityPtr &, const Position &);
			Game & getGame();
			const Game & getGame() const;
			void queueRemoval(const EntityPtr &);
			void queueRemoval(const TileEntityPtr &);
			void queueAddition(const EntityPtr &);
			void queueAddition(const TileEntityPtr &);
			void queue(std::function<void()>);
			void absorb(const EntityPtr &, const Position &);
			void setTile(Layer, Index row, Index column, TileID, bool run_helper = true);
			void setTile(Layer, const Position &, TileID, bool run_helper = true);
			void setTile(Layer, const Position &, const Identifier &, bool run_helper = true);
			TileID getTile(Layer, Index row, Index column) const;
			TileID getTile(Layer, const Position &) const;
			std::optional<TileID> tryTile(Layer, const Position &) const;
			std::optional<Position> getPathableAdjacent(const Position &) const;
			bool isPathable(const Position &) const;
			bool hasTileEntityAt(const Position &) const;
			void confirmGhosts();
			void damageGround(const Position &);
			Tileset & getTileset();
			/** Redoes the pathmap for the entire stored map, not just the visible ones! Can be very expensive. */
			void remakePathMap();
			void remakePathMap(const ChunkRange &);
			void remakePathMap(const ChunkPosition &);
			void markGenerated(const ChunkRange &);
			void markGenerated(ChunkPosition);
			bool isVisible(const Position &) const;
			bool hasTileEntity(GlobalID);
			Side getSide() const;
			inline void markGenerated(auto x, auto y) { generatedChunks.insert(ChunkPosition{x, y}); }
			inline auto pauseUpdates() { return Pauser(shared_from_this()); }
			inline bool isClient() const { return getSide() == Side::Client; }
			inline bool isServer() const { return getSide() == Side::Server; }

			virtual bool interactGround(const PlayerPtr &, const Position &, Modifiers);
			virtual void updateNeighbors(const Position &);
			/** Returns true iff something was done with the right click. */
			virtual bool rightClick(const Position &, double x, double y);
			/** Generates additional chunks for the infinite map after the initial worldgen of the realm. */
			virtual void generateChunk(const ChunkPosition &) {}

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
			bool focused = false;
			/** Whether to prevent updateNeighbors from running. */
			bool updatesPaused = false;

			Realm(Game &);
			Realm(Game &, RealmID, RealmType, Identifier tileset_id, int seed_);

			void initTexture();
			virtual void absorbJSON(const nlohmann::json &);
			virtual void toJSON(nlohmann::json &) const;

		private:
			Game &game;
			bool ticking = false;
			MTQueue<EntityPtr> entityRemovalQueue;
			MTQueue<EntityPtr> entityAdditionQueue;
			MTQueue<std::shared_ptr<TileEntity>> tileEntityRemovalQueue;
			MTQueue<std::shared_ptr<TileEntity>> tileEntityAdditionQueue;
			MTQueue<std::function<void()>> generalQueue;

			SharedRecursiveMutex entityMutex;
			SharedRecursiveMutex tileEntityMutex;

			// std::mutex entityRemovalQueueMutex;
			// std::mutex tileEntityRemovalQueueMutex;
			// std::mutex generalQueueMutex;

			void initRendererRealms();
			void initRendererTileProviders();

			bool isWalkable(Index row, Index column, const Tileset &);
			void setLayerHelper(Index row, Index col, bool should_mark_dirty = true);

			static BiomeType getBiome(uint32_t seed);

			std::atomic<std::thread::id> entityOwner;
			inline auto lockEntitiesUnique() {
				std::unique_lock lock(entityMutex);
				entityOwner = std::this_thread::get_id();
				return lock;
			}

			std::atomic<std::thread::id> tileEntityOwner;
			inline auto lockTileEntitiesUnique() {
				std::unique_lock lock(tileEntityMutex);
				tileEntityOwner = std::this_thread::get_id();
				return lock;
			}

			inline auto lockEntitiesShared() {
				try {
					return std::shared_lock(entityMutex);
				} catch (const std::system_error &) {
					std::cerr << "\e[31mThread " << std::this_thread::get_id() << " can't lock entity mutex held by " << entityOwner << "!\e[39m\n";
					throw;
				}
			}

			inline auto lockTileEntitiesShared() {
				try {
					return std::shared_lock(tileEntityMutex);
				} catch (const std::system_error &) {
					std::cerr << "\e[31mThread " << std::this_thread::get_id() << " can't lock tile entity mutex held by " << tileEntityOwner << "!\e[39m\n";
					throw;
				}
			}
	};

	void to_json(nlohmann::json &, const Realm &);

	using RealmPtr = std::shared_ptr<Realm>;
}
