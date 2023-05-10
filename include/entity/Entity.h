#pragma once

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "lib/Eigen.h"
#include <nlohmann/json.hpp>

#include "Direction.h"
#include "Position.h"
#include "Texture.h"
#include "Types.h"
#include "game/Agent.h"
#include "game/HasInventory.h"
#include "item/Item.h"

namespace Game3 {
	class Canvas;
	class Game;
	class Inventory;
	class Player;
	class Realm;
	class SpriteRenderer;

	struct EntityTexture: NamedRegisterable {
		Identifier textureID;
		uint8_t variety;
		EntityTexture(Identifier identifier_, Identifier texture_id, uint8_t variety_);
	};

	class Entity: public Agent, public HasInventory, public std::enable_shared_from_this<Entity> {
		public:
			constexpr static Slot DEFAULT_INVENTORY_SIZE = 30;
			/** The reciprocal of this is how many seconds it takes to move one square. */
			constexpr static float MAX_SPEED = 10.f;
			constexpr static HitPoints INVINCIBLE = 0;

			EntityType type;
			GlobalID globalID = -1;
			Position position {0, 0};
			RealmID realmID = 0;
			std::weak_ptr<Realm> weakRealm;
			Direction direction = Direction::Down;
			/** When the entity moves a square, its position field is immediately updated but this field is set to an offset
			 *  such that the sum of the new position and the offset is equal to the old offset. The offset is moved closer
			 *  to zero each tick to achieve smooth movement instead of teleportation from one tile to the next. */
			Eigen::Vector2f offset {0.f, 0.f};
			std::list<Direction> path;
			MoneyCount money = 0;
			HitPoints health = 0;

			~Entity() override = default;

			/** This won't call init() on the Entity. You need to do that yourself. */
			template <typename T = Entity, typename... Args>
			static std::shared_ptr<T> create(Args && ...args) {
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
			virtual void render(SpriteRenderer &);
			virtual void tick(Game &, float delta);
			/** Removes the entity from existence. */
			virtual void remove();
			/** Handles when the player interacts with the tile they're on and that tile contains this entity. Returns whether anything interesting happened. */
			virtual bool onInteractOn(const std::shared_ptr<Player> &) { return false; }
			/** Handles when the player interacts with the tile in front of them and that tile contains this entity. Returns whether anything interesting happened. */
			virtual bool onInteractNextTo(const std::shared_ptr<Player> &) { return false; }
			inline const Position::value_type & getRow()    const { return position.row;    }
			inline const Position::value_type & getColumn() const { return position.column; }
			inline Position::value_type & getRow()    { return position.row;    }
			inline Position::value_type & getColumn() { return position.column; }
			virtual void init(Game &);
			virtual void initAfterLoad(Game &) {}
			/** Returns whether the entity actually moved. */
			bool move(Direction);
			std::shared_ptr<Realm> getRealm() const override;
			inline const Position & getPosition() const override { return position; }
			Entity & setRealm(const Game &, RealmID);
			Entity & setRealm(const std::shared_ptr<Realm>);
			void focus(Canvas &, bool is_autofocus);
			void teleport(const Position &, bool clear_offset = true);
			virtual void teleport(const Position &, const std::shared_ptr<Realm> &);
			/** Returns the position of the tile in front of the entity. */
			Position nextTo() const;
			std::string debug() const;
			void queueForMove(const std::function<bool(const std::shared_ptr<Entity> &)> &);
			bool pathfind(const Position &start, const Position &goal, std::list<Direction> &);
			bool pathfind(const Position &goal);
			virtual float getSpeed() const { return MAX_SPEED; }
			virtual Glib::ustring getName() { return "Unknown Entity (" + std::string(type) + ')'; }
			Game & getGame();
			const Game & getGame() const;
			bool isVisible() const;
			void setHeldLeft(Slot);
			void setHeldRight(Slot);
			inline bool is(const Identifier &check) const { return type == check; }
			inline auto getHeldLeft()  const { return heldLeft.slot;  }
			inline auto getHeldRight() const { return heldRight.slot; }

			virtual void encode(Game &, Buffer &);
			/** More work needs to be done after this to initialize weakRealm. */
			virtual void decode(Game &, Buffer &);

		protected:
			Game *game = nullptr;
			std::shared_ptr<Texture> texture;
			int variety = 0;
			inline auto getHeldLeftTexture()  const { return heldLeft.texture;  }
			inline auto getHeldRightTexture() const { return heldRight.texture; }

			Entity() = delete;
			Entity(EntityType);

			bool canMoveTo(const Position &) const;
			/** A list of functions to call the next time the entity moves. The functions return whether they should be removed from the queue. */
			std::list<std::function<bool(const std::shared_ptr<Entity> &)>> moveQueue;
			std::shared_ptr<Texture> getTexture();

		private:
			struct Held {
				Slot slot = -1;
				std::shared_ptr<Texture> texture;
				float xOffset = 0.f;
				float yOffset = 0.f;
				inline explicit operator bool() const { return texture != nullptr; }
			};

			Held heldLeft;
			Held heldRight;

			void setHeld(Slot, Held &);
	};

	void to_json(nlohmann::json &, const Entity &);

	using EntityPtr = std::shared_ptr<Entity>;
}
