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
#include "game/HasInventory.h"
#include "game/HasRealm.h"
#include "item/Item.h"

namespace Game3 {
	class Canvas;
	class Game;
	class Inventory;
	class Player;
	class Realm;
	class SpriteRenderer;

	struct EntityTexture {
		Texture texture;
		uint8_t variety;
		EntityTexture(const Texture &texture_, uint8_t variety_): texture(texture_), variety(variety_) {}
	};

	class Entity: public HasInventory, public HasRealm, public std::enable_shared_from_this<Entity> {
		public:
			constexpr static Slot DEFAULT_INVENTORY_SIZE = 30;
			/** The reciprocal of this is how many seconds it takes to move one square. */
			constexpr static float MAX_SPEED = 15.f;
			constexpr static float MIN_SPEED = MAX_SPEED / 6.f;

			constexpr static EntityID  GANGBLANC_ID = 1;
			constexpr static EntityID       GRUM_ID = 2;
			constexpr static EntityID  VILLAGER1_ID = 3;
			constexpr static EntityID       ITEM_ID = 4;
			constexpr static EntityID BLACKSMITH_ID = 5;

			constexpr static EntityType    GENERIC_TYPE = 0;
			constexpr static EntityType     PLAYER_TYPE = 1;
			constexpr static EntityType   GATHERER_TYPE = 2;
			constexpr static EntityType   MERCHANT_TYPE = 3;
			constexpr static EntityType       ITEM_TYPE = 4;
			constexpr static EntityType BLACKSMITH_TYPE = 5;

			static std::unordered_map<EntityID, EntityTexture> textureMap;

			EntityType type = 0;
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

			~Entity() override = default;

			template <typename T = Entity, typename... Args>
			static std::shared_ptr<T> create(EntityID id, Args && ...args) {
				auto out = std::shared_ptr<T>(new T(id, std::forward<Args>(args)...));
				out->init();
				return out;
			}

			static std::shared_ptr<Entity> fromJSON(const nlohmann::json &);

			virtual void absorbJSON(const nlohmann::json &);
			virtual nlohmann::json toJSON() const;
			virtual bool isPlayer() const { return false; }
			virtual void render(SpriteRenderer &) const;
			virtual void tick(Game &, float delta);
			/** Removes the entity from existence. */
			virtual void remove();
			/** Handles when the player interacts with the tile they're on and that tile contains this entity. Returns whether anything interesting happened. */
			virtual bool onInteractOn(const std::shared_ptr<Player> &) { return false; }
			/** Handles when the player interacts with the tile in front of them and that tile contains this entity. Returns whether anything interesting happened. */
			virtual bool onInteractNextTo(const std::shared_ptr<Player> &) { return false; }
			inline EntityID id() const { return id_; }
			inline const Position::value_type & row()    const { return position.row;    }
			inline const Position::value_type & column() const { return position.column; }
			inline Position::value_type & row()    { return position.row;    }
			inline Position::value_type & column() { return position.column; }
			void id(EntityID);
			virtual void init();
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

		protected:
			Texture *texture = nullptr;
			int variety = 0;

			Entity() = delete;
			Entity(EntityID, EntityType);

			bool canMoveTo(const Position &) const;
			/** A list of functions to call the next time the entity moves. The functions return whether they should be removed from the queue. */
			std::list<std::function<bool(const std::shared_ptr<Entity> &)>> moveQueue;

		private:
			EntityID id_ = 0;
	};

	void to_json(nlohmann::json &, const Entity &);
}
