#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <set>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include <nlohmann/json_fwd.hpp>

#include "Types.h"
#include "error/MultipleFoundError.h"
#include "error/NoneFoundError.h"
#include "game/BiomeMap.h"
#include "game/TileProvider.h"
#include "packet/ChunkTilesPacket.h"
#include "packet/EntityPacket.h"
#include "packet/RealmNoticePacket.h"
#include "packet/TileEntityPacket.h"
#include "pipes/PipeLoader.h"
#include "threading/Lockable.h"
#include "threading/MTQueue.h"
#include "threading/SharedRecursiveMutex.h"
#include "tileentity/TileEntity.h"
#include "ui/ElementBufferedRenderer.h"
#include "ui/FluidRenderer.h"
#include "ui/Modifiers.h"
#include "util/GL.h"
#include "util/RWLock.h"
#include "container/WeakSet.h"

namespace Game3 {
	constexpr int64_t REALM_DIAMETER = 3;

	class Entity;
	class Game;
	class RemoteClient;
	class SpriteRenderer;
	class TextRenderer;

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
			RealmID id = -1;
			RealmType type;
			TileProvider tileProvider;
			PipeLoader pipeLoader;
			std::optional<std::array<std::array<std::array<ElementBufferedRenderer, LAYER_COUNT>, REALM_DIAMETER>, REALM_DIAMETER>> renderers;
			std::optional<std::array<std::array<FluidRenderer, REALM_DIAMETER>, REALM_DIAMETER>> fluidRenderers;
			Lockable<std::unordered_map<Position, TileEntityPtr>, SharedRecursiveMutex> tileEntities;
			Lockable<std::unordered_map<GlobalID, TileEntityPtr>> tileEntitiesByGID;
			Lockable<std::unordered_set<EntityPtr>, SharedRecursiveMutex> entities;
			Lockable<std::unordered_map<GlobalID, EntityPtr>> entitiesByGID;
			Lockable<WeakSet<Player>> players;
			nlohmann::json extraData;
			Position randomLand;
			/** Whether the realm's rendering should be affected by the day-night cycle. */
			bool outdoors = true;
			size_t ghostCount = 0;
			int64_t seed = 0;
			std::unordered_set<ChunkPosition> generatedChunks;
			Lockable<std::unordered_set<ChunkPosition>> visibleChunks;
			std::atomic_bool wakeupPending = false;
			std::atomic_bool snoozePending = false;

			std::atomic_bool reuploadPending = false;
			std::atomic_bool fluidReuploadPending = false;
			std::array<std::atomic_bool, LAYER_COUNT> layerReuploadPending{};
			std::atomic_bool renderersReady = false;

			Realm(const Realm &) = delete;
			Realm(Realm &&) = delete;

			virtual ~Realm() = default;

			Realm & operator=(const Realm &) = delete;
			Realm & operator=(Realm &&) = delete;

			template <typename T = Realm, typename... Args>
			static std::shared_ptr<T> create(Args && ...args) {
				return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
			}

			static std::shared_ptr<Realm> fromJSON(Game &, const nlohmann::json &, bool full_data = false);

			virtual void onFocus();
			virtual void onBlur();
			void createRenderers();
			void render(int width, int height, const Eigen::Vector2f &center, float scale, SpriteRenderer &, TextRenderer &, float game_time);
			/** Reuploads terrain in all layers. */
			void reupload();
			/** Reuploads terrain in one layer. The layer argument is 1-based. */
			void reupload(Layer);
			void reuploadFluids();
			EntityPtr addUnsafe(const EntityPtr &, const Position &);
			EntityPtr add(const EntityPtr &, const Position &);
			TileEntityPtr add(const TileEntityPtr &);
			TileEntityPtr addUnsafe(const TileEntityPtr &);
			void initEntities();
			void tick(float delta);
			std::vector<EntityPtr> findEntities(const Position &);
			std::vector<EntityPtr> findEntities(const Position &, const EntityPtr &except);
			EntityPtr findEntity(const Position &);
			EntityPtr findEntity(const Position &, const EntityPtr &except);
			TileEntityPtr tileEntityAt(const Position &);
			void remove(EntityPtr);
			void removeSafe(const EntityPtr &);
			void remove(TileEntityPtr, bool run_helper = true);
			void removeSafe(const TileEntityPtr &);
			void onMoved(const EntityPtr &, const Position &);
			Game & getGame();
			const Game & getGame() const;
			void queueRemoval(const EntityPtr &);
			void queueRemoval(const TileEntityPtr &);
			void queueDestruction(const EntityPtr &);
			void queueDestruction(const TileEntityPtr &);
			void queuePlayerRemoval(const PlayerPtr &);
			void queueAddition(const EntityPtr &, const Position &);
			void queueAddition(const TileEntityPtr &);
			void queue(std::function<void()>);
			void absorb(const EntityPtr &, const Position &);
			void setTile(Layer, Index row, Index column, TileID, bool run_helper = true, bool generating = false);
			void setTile(Layer, const Position &, TileID, bool run_helper = true, bool generating = false);
			void setTile(Layer, const Position &, const Identifier &, bool run_helper = true, bool generating = false);
			void setFluid(const Position &, FluidTile, bool run_helper = true, bool generating = false);
			void setFluid(const Position &, const Identifier &, FluidLevel, bool run_helper = true, bool generating = false);
			bool hasFluid(const Position &, FluidLevel minimum = 1);
			TileID getTile(Layer, const Position &) const;
			std::optional<TileID> tryTile(Layer, const Position &) const;
			std::optional<FluidTile> tryFluid(const Position &) const;
			bool middleEmpty(const Position &);
			std::optional<Position> getPathableAdjacent(const Position &) const;
			bool isPathable(const Position &) const;
			bool hasTileEntityAt(const Position &) const;
			void confirmGhosts();
			void damageGround(const Position &);
			Tileset & getTileset();
			/** Redoes the pathmap for the entire stored map, not just the visible ones! Can be very expensive. */
			void remakePathMap();
			void remakePathMap(const ChunkRange &);
			void remakePathMap(ChunkPosition);
			void remakePathMap(Position);
			void markGenerated(const ChunkRange &);
			void markGenerated(ChunkPosition);
			bool isVisible(const Position &);
			bool hasTileEntity(GlobalID);
			bool hasEntity(GlobalID);
			EntityPtr getEntity(GlobalID);
			TileEntityPtr getTileEntity(GlobalID);
			Side getSide() const;
			/** Client-side. */
			std::set<ChunkPosition> getMissingChunks() const;
			void addPlayer(const PlayerPtr &);
			void removePlayer(const PlayerPtr &);
			void sendTo(RemoteClient &);
			void requestChunk(ChunkPosition, const std::shared_ptr<RemoteClient> &);
			/** Removes the entity from entitiesByChunk. */
			void detach(const EntityPtr &, ChunkPosition);
			/** Removes the entity from entitiesByChunk based on the entitiy's current chunk position. */
			void detach(const EntityPtr &);
			/** Adds the entity to entitiesByChunk. */
			void attach(const EntityPtr &);
			std::shared_ptr<Lockable<std::unordered_set<EntityPtr>>> getEntities(ChunkPosition);
			/** Removes the tile entity from tileEntitiesByChunk. */
			void detach(const TileEntityPtr &);
			/** Adds the tile entity to tileEntitiesByChunk. */
			void attach(const TileEntityPtr &);
			std::shared_ptr<Lockable<std::unordered_set<TileEntityPtr>>> getTileEntities(ChunkPosition);
			void sendToMany(const std::unordered_set<std::shared_ptr<RemoteClient>> &, ChunkPosition);
			void sendToOne(RemoteClient &, ChunkPosition);
			void recalculateVisibleChunks();
			void queueReupload();
			void queueReupload(Layer);
			void queueReuploadFluids();
			void queueReuploadAll();

			inline const auto & getPlayers() const { return players; }
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

			/** Full data doesn't include terrain, entities or tile entities. */
			virtual void toJSON(nlohmann::json &, bool full_data) const;

			template <typename T, typename... Args>
			std::shared_ptr<T> spawn(const Position &position, Args && ...args) {
				Game &game_ref = getGame();
				auto entity = T::create(game_ref, std::forward<Args>(args)...);
				entity->spawning = true;
				entity->setRealm(shared_from_this());
				entityInitializationQueue.emplace(entity, position);
				return entity;
			}

			template <typename T>
			std::shared_ptr<T> getTileEntity() const {
				std::shared_ptr<T> out;
				for (const auto &[index, tile_entity]: tileEntities) {
					if (auto cast = std::dynamic_pointer_cast<T>(tile_entity)) {
						if (out)
							throw MultipleFoundError("Multiple tile entities of type " + std::string(typeid(T).name()) + " found");
						out = cast;
					}
				}
				if (!out)
					throw NoneFoundError("No tile entities of type " + std::string(typeid(T).name()) + " found");
				return out;
			}

			template <typename T, typename P>
			std::shared_ptr<T> getTileEntity(const P &predicate) const {
				std::shared_ptr<T> out;
				for (const auto &[index, tile_entity]: tileEntities) {
					if (auto cast = std::dynamic_pointer_cast<T>(tile_entity)) {
						if (predicate(cast)) {
							if (out)
								throw MultipleFoundError("Multiple tile entities of type " + std::string(typeid(T).name()) + " found");
							out = cast;
						}
					}
				}
				if (!out)
					throw NoneFoundError("No tile entities of type " + std::string(typeid(T).name()) + " found");
				return out;
			}

			template <typename T>
			std::shared_ptr<T> closestTileEntity(const Position &position) const {
				double minimum_distance = INFINITY;
				std::shared_ptr<T> out;
				for (const auto &[index, entity]: tileEntities) {
					if (auto cast = std::dynamic_pointer_cast<T>(entity)) {
						const double distance = entity->position.distance(position);
						if (distance < minimum_distance) {
							minimum_distance = distance;
							out = cast;
						}
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
				for (const auto &[index, entity]: tileEntities) {
					if (auto cast = std::dynamic_pointer_cast<T>(entity)) {
						const double distance = entity->position.distance(position);
						if (predicate(cast) && distance < minimum_distance) {
							minimum_distance = distance;
							out = cast;
						}
					}
				}
				if (!out)
					throw std::runtime_error("No tile entities of type " + std::string(typeid(T).name()) + " found");
				return out;
			}

			friend class MainWindow;

		protected:
			std::atomic_bool focused = false;
			/** Whether to prevent updateNeighbors from running. */
			bool updatesPaused = false;

			Realm(Game &);
			Realm(Game &, RealmID, RealmType, Identifier tileset_id, int64_t seed_);

			void initTexture();

			/** Full data doesn't include terrain, entities or tile entities. */
			virtual void absorbJSON(const nlohmann::json &, bool full_data);

		private:
			struct ChunkPackets {
				ChunkTilesPacket tilePacket;
				std::vector<EntityPacket> entityPackets;
				std::vector<TileEntityPacket> tileEntityPackets;
			};

			Game &game;
			bool ticking = false;
			MTQueue<std::weak_ptr<Entity>> entityRemovalQueue;
			MTQueue<std::weak_ptr<Entity>> entityDestructionQueue;
			MTQueue<std::pair<std::shared_ptr<Entity>, Position>> entityAdditionQueue;
			MTQueue<std::pair<EntityPtr, Position>> entityInitializationQueue;
			MTQueue<std::weak_ptr<TileEntity>> tileEntityRemovalQueue;
			MTQueue<std::weak_ptr<TileEntity>> tileEntityDestructionQueue;
			MTQueue<std::weak_ptr<TileEntity>> tileEntityAdditionQueue;
			MTQueue<std::weak_ptr<Player>> playerRemovalQueue;
			MTQueue<std::function<void()>> generalQueue;
			Lockable<std::unordered_map<ChunkPosition, std::shared_ptr<Lockable<std::unordered_set<EntityPtr>>>>> entitiesByChunk;
			Lockable<std::unordered_map<ChunkPosition, std::shared_ptr<Lockable<std::unordered_set<TileEntityPtr>>>>> tileEntitiesByChunk;

			friend class ServerGame;

			Lockable<std::map<ChunkPosition, WeakSet<RemoteClient>>> chunkRequests;

			SharedRecursiveMutex tileEntityMutex;

			void initRendererRealms();
			void initRendererTileProviders();
			bool isWalkable(Index row, Index column, const Tileset &);
			void setLayerHelper(Index row, Index col, bool should_mark_dirty = true);
			ChunkPackets getChunkPackets(ChunkPosition);
			void initEntity(const EntityPtr &, const Position &);

			static BiomeType getBiome(int64_t seed);
	};

	using RealmPtr = std::shared_ptr<Realm>;
}
