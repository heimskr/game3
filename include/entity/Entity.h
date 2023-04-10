#pragma once

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include <Eigen/Eigen>
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
		Texture texture;
		uint8_t variety;
		EntityTexture(Texture texture_, uint8_t variety_):
			NamedRegisterable(texture_.identifier),
			texture(std::move(texture_)),
			variety(variety_) {}
	};

	class Entity: public Agent, public HasInventory, public std::enable_shared_from_this<Entity> {
		public:
			constexpr static Slot DEFAULT_INVENTORY_SIZE = 30;
			/** The reciprocal of this is how many seconds it takes to move one square. */
			constexpr static float MAX_SPEED = 15.f;
			constexpr static float MIN_SPEED = MAX_SPEED / 6.f;

			EntityType type;
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

			template <typename T = Entity, typename... Args>
			static std::shared_ptr<T> create(Game &game, Args && ...args) {
				auto out = std::shared_ptr<T>(new T(game, std::forward<Args>(args)...));
				out->health = out->maxHealth();
				out->init(game);
				return out;
			}

			static std::shared_ptr<Entity> fromJSON(Game &, const nlohmann::json &);

			virtual void absorbJSON(const nlohmann::json &);
			virtual void toJSON(nlohmann::json &) const;
			virtual bool isPlayer() const { return false; }
			/** Returns the maximum number of hitpoints this entity can have. If 0, the entity is invincible. */
			virtual HitPoints maxHealth() const { return 0; }
			bool isInvincible() const { return maxHealth() == 0; }
			virtual void render(SpriteRenderer &) const;
			virtual void tick(Game &, float delta);
			/** Removes the entity from existence. */
			virtual void remove();
			/** Handles when the player interacts with the tile they're on and that tile contains this entity. Returns whether anything interesting happened. */
			virtual bool onInteractOn(const std::shared_ptr<Player> &) { return false; }
			/** Handles when the player interacts with the tile in front of them and that tile contains this entity. Returns whether anything interesting happened. */
			virtual bool onInteractNextTo(const std::shared_ptr<Player> &) { return false; }
			inline const Position::value_type & row()    const { return position.row;    }
			inline const Position::value_type & column() const { return position.column; }
			inline Position::value_type & row()    { return position.row;    }
			inline Position::value_type & column() { return position.column; }
			virtual void init(Game &);
			virtual void initAfterLoad(Game &) {}
			/** Returns whether the entity actually moved. */
			bool move(Direction);
			std::shared_ptr<Realm> getRealm() const override;
			const Position & getPosition() const override { return position; }
			Entity & setRealm(const Game &, RealmID);
			Entity & setRealm(const std::shared_ptr<Realm>);
			void focus(Canvas &, bool is_autofocus);
			void teleport(const Position &, bool clear_offset = true);
			virtual void teleport(const Position &, const std::shared_ptr<Realm> &);
			void teleport(Index, const std::shared_ptr<Realm> &);
			/** Returns the position of the tile in front of the entity. */
			Position nextTo() const;
			std::string debug() const;
			void queueForMove(const std::function<bool(const std::shared_ptr<Entity> &)> &);
			bool pathfind(const Position &start, const Position &goal, std::list<Direction> &);
			bool pathfind(const Position &goal);
			virtual float getSpeed() const { return MAX_SPEED; }
			virtual Glib::ustring getName() { return "Unknown Entity (" + std::to_string(type) + ')'; }
			Game & getGame();
			const Game & getGame() const;

		protected:
			Texture *texture = nullptr;
			int variety = 0;

			Entity() = delete;
			Entity(EntityType);

			bool canMoveTo(const Position &) const;
			/** A list of functions to call the next time the entity moves. The functions return whether they should be removed from the queue. */
			std::list<std::function<bool(const std::shared_ptr<Entity> &)>> moveQueue;
	};

	void to_json(nlohmann::json &, const Entity &);

	using EntityPtr = std::shared_ptr<Entity>;
}
