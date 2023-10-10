#pragma once

#include "lib/Eigen.h"
#include <nlohmann/json_fwd.hpp>

#include "Direction.h"
#include "Position.h"
#include "graphics/Texture.h"
#include "Types.h"
#include "container/WeakSet.h"
#include "game/Agent.h"
#include "game/ChunkPosition.h"
#include "game/HasInventory.h"
#include "game/MovementContext.h"
#include "item/Item.h"
#include "threading/Atomic.h"
#include "threading/HasMutex.h"
#include "threading/Lockable.h"
#include "threading/LockableSharedPtr.h"
#include "threading/LockableWeakPtr.h"
#include "threading/SharedRecursiveMutex.h"
#include "ui/Modifiers.h"

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
	class Canvas;
	class Game;
	class Inventory;
	class Player;
	class Realm;
	class RemoteClient;
	class SpriteRenderer;
	class TextRenderer;
	class TileEntity;

	struct EntityTexture: NamedRegisterable {
		Identifier textureID;
		uint8_t variety;
		EntityTexture(Identifier identifier_, Identifier texture_id, uint8_t variety_);
	};

	class Entity: public Agent, public HasInventory, public HasMutex<SharedRecursiveMutex> {
		public:
			constexpr static Slot DEFAULT_INVENTORY_SIZE = 30;
			/** The reciprocal of this is how many seconds it takes to move one square. */
			constexpr static float MAX_SPEED = 10.f;
			constexpr static HitPoints INVINCIBLE = 0;

			EntityType type;
			Lockable<Position> position{0, 0};
			Atomic<RealmID> realmID = 0;
			LockableWeakPtr<Realm> weakRealm;
			Atomic<Direction> direction = Direction::Down;
			/** When the entity moves a square, its position field is immediately updated but this field is set to an offset
			 *  such that the sum of the new position and the offset is equal to the old offset. The offset is moved closer
			 *  to zero each tick to achieve smooth movement instead of teleportation from one tile to the next. */
			Lockable<Offset> offset;
			Atomic<float> zSpeed = 0.f;
			Lockable<std::list<Direction>> path;
			Atomic<MoneyCount> money = 0;
			Atomic<HitPoints> health = 0;
			Lockable<WeakSet<Entity>> visibleEntities;
			Lockable<WeakSet<Player>> visiblePlayers;
			/** Set when an entity is beginning to teleport so that an EntityMovedPacket can be sent with the proper realm ID
			 *  before the actual realm switch has occurred. */
			Atomic<RealmID> nextRealm = 0;
			Atomic<bool> spawning = false;
			/** Whether the entity is currently teleporting to its first position on realm change. */
			Atomic<bool> firstTeleport = false;
			Atomic<RealmID> inLimboFor{-1};
			Identifier customTexture;

			virtual void destroy();

			/** This won't call init() on the Entity. You need to do that yourself. */
			template <typename T = Entity, typename... Args>
			static std::shared_ptr<T> create(Args &&...args) {
				auto out = std::shared_ptr<T>(new T(std::forward<Args>(args)...));
				out->health = out->maxHealth();
				return out;
			}

			static std::shared_ptr<Entity> fromJSON(Game &, const nlohmann::json &);

			virtual void absorbJSON(Game &, const nlohmann::json &);
			virtual void toJSON(nlohmann::json &) const;
			virtual bool isPlayer() const { return false; }
			/** Returns the maximum number of hitpoints this entity can have. If 0, the entity is invincible. */
			virtual HitPoints maxHealth() const { return 0; }
			bool isInvincible() const { return maxHealth() == INVINCIBLE; }
			virtual void render(SpriteRenderer &, TextRenderer &);
			virtual void tick(Game &, float delta);
			/** Removes the entity from existence. */
			virtual void remove();
			inline const Position::value_type & getRow()    const { return position.row;    }
			inline const Position::value_type & getColumn() const { return position.column; }
			inline Position::value_type getRow()    { auto lock = position.sharedLock(); return position.row;    }
			inline Position::value_type getColumn() { auto lock = position.sharedLock(); return position.column; }
			virtual void init(Game &);
			virtual void initAfterLoad(Game &) {}
			/** Returns whether the entity actually moved. */
			virtual bool move(Direction, MovementContext);
			bool move(Direction direction);
			std::shared_ptr<Realm> getRealm() const override final;
			inline Position getPosition() const override { return position.copyBase(); }
			Entity & setRealm(const Game &, RealmID);
			Entity & setRealm(const std::shared_ptr<Realm>);
			void focus(Canvas &, bool is_autofocus);
			/** Returns whether the entity moved to a new chunk. */
			bool teleport(const Position &, MovementContext = {});
			virtual void teleport(const Position &, const std::shared_ptr<Realm> &, MovementContext);
			void teleport(const Position &position, const std::shared_ptr<Realm> &realm) { teleport(position, realm, MovementContext{}); }
			/** Returns the position of the tile in front of the entity. */
			Position nextTo() const;
			std::string debug() const;
			void queueForMove(const std::function<bool(const std::shared_ptr<Entity> &)> &);
			void queueDestruction();
			PathResult pathfind(const Position &start, const Position &goal, std::list<Direction> &, size_t loop_max = 1'000);
			bool pathfind(const Position &goal, size_t loop_max = 1'000);
			virtual float getSpeed() const { return MAX_SPEED; }
			std::string getName() const override { return "Unknown Entity (" + std::string(type) + ')'; }
			Game & getGame();
			Game & getGame() const;
			bool isVisible() const;
			bool setHeldLeft(Slot);
			bool setHeldRight(Slot);
			inline auto getHeldLeft()  const { return heldLeft.slot;  }
			inline auto getHeldRight() const { return heldRight.slot; }
			void unhold(Slot);
			Side getSide() const override final;
			Type getAgentType() const override final { return Agent::Type::Entity; }
			void inventoryUpdated() override;
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
			void clearQueues();
			bool isInLimbo() const;
			virtual float getJumpSpeed() const { return 8.f; }
			void changeTexture(const Identifier &);
			virtual int getZIndex() const { return 0; }

			virtual void encode(Buffer &);
			/** More work needs to be done after this to initialize weakRealm. */
			virtual void decode(Buffer &);

			void sendTo(RemoteClient &, UpdateCounter threshold = 0);
			void sendToVisible();

			template <template <typename...> typename T>
			T<Direction> copyPath() {
				auto lock = path.sharedLock();
				return {path.begin(), path.end()};
			}

		protected:
			Game *game = nullptr;
			LockableSharedPtr<Texture> texture;
			int variety = 0;
			float renderHeight = 16.f;
			std::atomic_bool awaitingDestruction = false;

			Entity() = delete;
			Entity(EntityType);

			bool canMoveTo(const Position &) const;
			/** A list of functions to call the next time the entity moves. Each function returns whether it should be removed from the queue. */
			Lockable<std::list<std::function<bool(const std::shared_ptr<Entity> &)>>> moveQueue;
			std::shared_ptr<Texture> getTexture();
			inline auto getHeldLeftTexture()  const { return heldLeft.texture;  }
			inline auto getHeldRightTexture() const { return heldRight.texture; }

			std::shared_ptr<Agent> getSharedAgent() override { return shared_from_this(); }

		private:
			struct Held {
				Slot slot = -1;
				bool isLeft;
				float xOffset = 0.f;
				float yOffset = 0.f;
				std::shared_ptr<Texture> texture;
				Held() = delete;
				Held(bool is_left): isLeft(is_left) {}
				inline explicit operator bool() const { return texture != nullptr; }
			};

			Atomic<GlobalID> otherEntityToLock = -1;

			Held heldLeft {true};
			Held heldRight{false};
			/** The set of all players who have been sent a packet about the entity's current path. Governed by pathSeersMutex */
			Lockable<WeakSet<Player>> pathSeers;

			bool setHeld(Slot, Held &);
	};

	void to_json(nlohmann::json &, const Entity &);

	using EntityPtr = std::shared_ptr<Entity>;
}
