#pragma once

#include "container/WeakSet.h"
#include "game/Agent.h"
#include "game/HasDimensions.h"
#include "game/HasInventory.h"
#include "game/Tickable.h"
#include "graphics/Texture.h"
#include "item/Item.h"
#include "packet/EntityMoneyChangedPacket.h"
#include "threading/Atomic.h"
#include "threading/HasMutex.h"
#include "threading/Lockable.h"
#include "threading/LockableSharedPtr.h"
#include "threading/LockableWeakPtr.h"
#include "threading/SharedRecursiveMutex.h"
#include "types/ChunkPosition.h"
#include "types/Direction.h"
#include "types/MovementContext.h"
#include "types/Position.h"
#include "types/TickArgs.h"
#include "types/Types.h"
#include "ui/Modifiers.h"

#include <nlohmann/json_fwd.hpp>

#include <atomic>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace Game3 {
	class Game;
	class Inventory;
	class Player;
	class Realm;
	class GenericClient;
	class TileEntity;
	class Window;
	struct RendererContext;

	enum class RideType {
		Visible, Hidden
	};

	struct EntityTexture: NamedRegisterable {
		Identifier textureID;
		uint8_t variety;
		EntityTexture(Identifier identifier_, Identifier texture_id, uint8_t variety_);
	};

	class Entity: public Agent, public HasDimensions, public HasInventory, public Tickable, public HasMutex<SharedRecursiveMutex> {
		public:
			constexpr static Slot DEFAULT_INVENTORY_SIZE = 30;
			/** The reciprocal of this is how many seconds it takes to move one square. */
			constexpr static float MAX_SPEED = 10.f;

			EntityType type;
			Lockable<Position> position{0, 0};
			Atomic<RealmID> realmID = 0;
			LockableWeakPtr<Realm> weakRealm;
			Atomic<Direction> direction = Direction::Down;
			/** When the entity moves a square, its position field is immediately updated but this field is set to an offset
			 *  such that the sum of the new position and the offset is equal to the old offset. The offset is moved closer
			 *  to zero each tick to achieve smooth movement instead of teleportation from one tile to the next. */
			Lockable<Vector3> offset;
			/** Only the z component is handled in the default Entity tick method. */
			Lockable<Vector3> velocity;
			Lockable<std::list<Direction>> path;
			Lockable<WeakSet<Entity>> visibleEntities;
			Lockable<WeakSet<Player>> visiblePlayers;
			/** Set when an entity is beginning to teleport so that an EntityMovedPacket can be sent with the proper realm ID
			 *  before the actual realm switch has occurred. */
			Atomic<RealmID> nextRealm = 0;
			Atomic<bool> spawning = false;
			/** Whether the entity is currently teleporting to its first position on realm change. */
			Atomic<bool> firstTeleport = false;
			Atomic<RealmID> inLimboFor{0};
			Identifier customTexture;
			Lockable<std::optional<Position>> pathfindGoal;
			Atomic<float> age;

			virtual void destroy();

			/** This won't call init() on the Entity. You need to do that yourself. */
			template <typename T = Entity, typename... Args>
			static std::shared_ptr<T> create(Args &&...args) {
				auto out = std::shared_ptr<T>(new T(std::forward<Args>(args)...));
				out->onCreate();
				return out;
			}

			static std::shared_ptr<Entity> fromJSON(const std::shared_ptr<Game> &, const nlohmann::json &);
			static std::shared_ptr<Entity> fromBuffer(const std::shared_ptr<Game> &, Buffer &);

			static std::string getSQL();

			virtual void absorbJSON(const std::shared_ptr<Game> &, const nlohmann::json &);
			virtual void toJSON(nlohmann::json &) const;
			virtual void init(const std::shared_ptr<Game> &);
			virtual void render(const RendererContext &);
			virtual void renderUpper(const RendererContext &);
			virtual void renderLighting(const RendererContext &);
			virtual void tick(const TickArgs &);
			/** Whether the entity should be included in save data. */
			virtual bool shouldPersist() const { return true; }
			virtual void onCreate() {}
			virtual void onSpawn() {}
			/** Called at the beginning of destroy(). */
			virtual void onDestroy() {}
			std::string getName() const override { return "Unknown Entity (" + std::string(type) + ')'; }

			virtual bool isPlayer() const { return false; }
			virtual bool isSpawnableMonster() const { return false; }
			/** Removes the entity from existence. */
			virtual void remove();
			/** Updates the offset and position of the entity's rider. */
			void updateRider(const std::shared_ptr<Entity> &rider);
			/** Updates the position of the entity's rider. Entity subclasses can override this to adjust the position differently, but this is rarely necessary. */
			virtual void updateRiderPosition(const std::shared_ptr<Entity> &rider);
			/** Updates the offset of the entity's rider. Entity subclasses can override this to adjust the offset differently. */
			virtual void updateRiderOffset(const std::shared_ptr<Entity> &rider);
			virtual void setRider(const std::shared_ptr<Entity> &);
			virtual void setRidden(const std::shared_ptr<Entity> &);
			inline std::shared_ptr<Entity> getRider()  const { return weakRider.lock();  }
			inline std::shared_ptr<Entity> getRidden() const { return weakRidden.lock(); }
			virtual RideType getRideType() const { return RideType::Visible; }
			inline Position::IntType getRow()    { auto lock = position.sharedLock(); return position.row;    }
			inline Position::IntType getColumn() { auto lock = position.sharedLock(); return position.column; }
			virtual void initAfterLoad(Game &) {}
			/** Returns whether the entity actually moved. */
			virtual bool move(Direction, MovementContext);
			bool move(Direction direction);
			/** Called whenever the riding entity tries to move. Returns whether the move request was accepted. */
			virtual bool moveFromRider(const std::shared_ptr<Entity> &rider, Direction, MovementContext);
			bool moveFromRider(const std::shared_ptr<Entity> &rider, Direction);
			std::shared_ptr<Realm> getRealm() const final;
			Position getPosition() const override { return position.copyBase(); }
			inline Direction getDirection() const { return direction.load(); }
			Entity & setRealm(const Game &, RealmID);
			Entity & setRealm(const std::shared_ptr<Realm>);
			void focus(Window &, bool is_autofocus);
			/** Returns whether the entity moved to a new chunk. */
			bool teleport(const Position &, MovementContext = {});
			virtual void teleport(const Position &, const std::shared_ptr<Realm> &, MovementContext);
			void teleport(const Position &position, const std::shared_ptr<Realm> &realm) { teleport(position, realm, MovementContext{}); }
			/** Returns the position of the tile in front of the entity. */
			Position nextTo() const;
			std::string debug() const;
			/** The second parameter to the function indicates whether the call is because the queue is being cleared, rather than because the entity actually moved.
			 *  The function returns true if it should be removed from the move queue. */
			void queueForMove(std::function<bool(const std::shared_ptr<Entity> &, bool)>);
			void queueDestruction();
			PathResult pathfind(const Position &start, const Position &goal, std::list<Direction> &, size_t loop_max = 1'000);
			bool pathfind(const Position &goal, size_t loop_max = 1'000);
			virtual float getMovementSpeed() const;
			std::shared_ptr<Game> getGame() const override;
			bool isVisible() const;
			bool isInFluid() const;
			bool setHeldLeft(Slot);
			bool setHeldRight(Slot);
			inline Slot getHeldLeft()  const { return heldLeft.slot;  }
			inline Slot getHeldRight() const { return heldRight.slot; }
			void unhold(Slot);
			Side getSide() const final;
			Type getAgentType() const final { return Agent::Type::Entity; }
			void inventoryUpdated(InventoryID) override;
			ChunkPosition getChunk() const;
			bool canSee(RealmID, const Position &) const;
			bool canSee(const Entity &) const;
			bool canSee(const TileEntity &) const;
			virtual void movedToNewChunk(const std::optional<ChunkPosition> &);
			bool hasSeenPath(const PlayerPtr &);
			void setSeenPath(const PlayerPtr &, bool seen = true);
			/** Returns the number of visible sets the entity was removed from. */
			size_t removeVisible(const std::weak_ptr<Entity> &);
			/** Returns the number of visible sets the player was removed from. */
			size_t removeVisible(const std::weak_ptr<Player> &);
			void calculateVisibleEntities();
			virtual void jump();
			void clearOffset();
			inline bool is(const Identifier &check) const { return type == check; }
			std::shared_ptr<Entity> getSelf();
			std::weak_ptr<Entity> getWeakSelf();
			void clearQueues();
			bool isInLimbo() const;
			virtual float getJumpSpeed() const { return 8.f; }
			void changeTexture(const Identifier &);
			virtual int getZIndex() const { return 0; }
			virtual Identifier getMilk() const { return {}; }
			ItemStackPtr getHeld(Hand) const;
			Slot getHeldSlot(Hand) const;
			Slot getActiveSlot() const;
			bool isOffsetZero() const;
			virtual Vector3 getOffset() const;
			virtual void setOffset(const Vector3 &);
			virtual bool canSpawnAt(const Place &) const;
			/** Returns whether the entity is on the ground and not in the air. */
			virtual bool isGrounded() const;
			virtual bool isAffectedByKnockback() const;

			virtual void encode(Buffer &);
			/** More work needs to be done after this to initialize weakRealm. */
			virtual void decode(Buffer &);

			inline MoneyCount getMoney() const { return money; }
			virtual void setMoney(MoneyCount);
			virtual void broadcastMoney();

			void sendTo(GenericClient &, UpdateCounter threshold = 0);
			void sendToVisible();

			template <template <typename...> typename T>
			T<Direction> copyPath() {
				auto lock = path.sharedLock();
				return {path.begin(), path.end()};
			}

			template <Duration D>
			requires (!std::is_same_v<D, std::chrono::nanoseconds>)
			Tick enqueueTick(D delay) {
				return enqueueTick(std::chrono::duration_cast<std::chrono::nanoseconds>(delay));
			}

			Tick enqueueTick(std::chrono::nanoseconds);
			Tick enqueueTick() override;

		protected:
			struct Held {
				Slot slot = -1;
				bool isLeft;
				float offsetX = 0.f;
				float offsetY = 0.f;
				std::shared_ptr<Texture> texture;
				Held() = delete;
				Held(bool is_left): isLeft(is_left) {}
				inline explicit operator bool() const { return texture != nullptr; }
			};

			mutable std::weak_ptr<Game> weakGame;
			LockableSharedPtr<Texture> texture;
			int variety = 0;
			float renderHeight = 16.f;
			std::atomic_bool awaitingDestruction = false;
			Atomic<MoneyCount> money = 0;
			std::weak_ptr<Entity> weakRider;
			std::weak_ptr<Entity> weakRidden;
			Held heldLeft {true};
			Held heldRight{false};

			Entity() = delete;
			Entity(EntityType type);

			virtual bool canMoveTo(const Place &) const;
			/** A list of functions to call the next time the entity moves. Each function returns whether it should be removed from the queue. */
			Lockable<std::list<std::function<bool(const std::shared_ptr<Entity> &, bool)>>> moveQueue;
			virtual std::shared_ptr<Texture> getTexture();
			inline auto getHeldLeftTexture()  const { return heldLeft.texture;  }
			inline auto getHeldRightTexture() const { return heldRight.texture; }

			std::shared_ptr<Agent> getSharedAgent() override { return shared_from_this(); }

			std::function<void(const TickArgs &)> getTickFunction();

		private:

			Atomic<GlobalID> otherEntityToLock = -1;

			/** The set of all players who have been sent a packet about the entity's current path. */
			Lockable<WeakSet<Player>> pathSeers;

			bool setHeld(Slot, Held &);
	};

	void to_json(nlohmann::json &, const Entity &);

	using EntityPtr = std::shared_ptr<Entity>;
	using WeakEntityPtr = std::weak_ptr<Entity>;
}
