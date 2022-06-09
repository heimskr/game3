#pragma once

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include <nanogui/common.h>
#include <nlohmann/json.hpp>

#include "Direction.h"
#include "Position.h"
#include "Texture.h"
#include "Types.h"
#include "game/HasInventory.h"
#include "game/HasRealm.h"
#include "game/Item.h"

namespace Game3 {
	class Canvas;
	class Game;
	class Inventory;
	class Player;
	class Realm;
	class SpriteRenderer;

	class Entity: public HasInventory, public HasRealm, public std::enable_shared_from_this<Entity> {
		public:
			constexpr static Slot DEFAULT_INVENTORY_SIZE = 20;
			constexpr static EntityID GANGBLANC = 1;
			constexpr static EntityID GRUM = 2;
			constexpr static EntityID ITEM = 3;

			static std::unordered_map<EntityID, Texture> textureMap;

			/** (row, column) */
			Position position {0, 0};
			RealmID realmID = 0;
			std::weak_ptr<Realm> weakRealm;
			Direction direction = Direction::Down;
			/** The reciprocal of this is how many seconds it takes to move one square. */
			float speed = 10.f;
			/** When the entity moves a square, its position field is immediately updated but this field is set to an offset
			 *  such that the sum of the new position and the offset is equal to the old offset. The offset is moved closer
			 *  to zero each tick to achieve smooth movement instead of teleportation from one tile to the next. */
			nanogui::Vector2f offset {0.f, 0.f};

			~Entity() override = default;

			template <typename T>
			static std::shared_ptr<T> create(EntityID id) {
				auto out = std::shared_ptr<T>(new T(id));
				out->init();
				return out;
			}

			static std::shared_ptr<Entity> fromJSON(const nlohmann::json &);

			virtual void absorbJSON(const nlohmann::json &);
			virtual nlohmann::json toJSON() const;
			virtual bool isPlayer() const { return false; }
			virtual void render(SpriteRenderer &) const;
			virtual void tick(float delta);
			/** Removes the entity from existence. */
			virtual void remove();
			/** Handles when the player interacts with the tile they're on and that tile contains this entity. */
			virtual void onInteractOn(const std::shared_ptr<Player> &) {}
			/** Handles when the player interacts with the tile in front of them and that tile contains this entity. */
			virtual void onInteractNextTo(const std::shared_ptr<Player> &) {}
			inline EntityID id() const { return id_; }
			inline const Position::value_type & row()    const { return position.row;    }
			inline const Position::value_type & column() const { return position.column; }
			inline Position::value_type & row()    { return position.row;    }
			inline Position::value_type & column() { return position.column; }
			void id(EntityID);
			virtual void init();
			void move(Direction);
			std::shared_ptr<Realm> getRealm() const override;
			const Position & getPosition() const override { return position; }
			void setRealm(const Game &, RealmID);
			void setRealm(const std::shared_ptr<Realm>);
			void focus(Canvas &, bool is_autofocus);
			void teleport(const Position &, bool clear_offset = true);
			virtual void teleport(const Position &, const std::shared_ptr<Realm> &);
			void teleport(Index, const std::shared_ptr<Realm> &);
			/** Returns the position of the tile in front of the entity. */
			Position nextTo() const;
			std::string debug() const;
			void queueForMove(const std::function<bool(const std::shared_ptr<Entity> &)> &);

		protected:
			Entity() = delete;
			Entity(EntityID id__): id_(id__) {}
			Texture *texture = nullptr;
			bool canMoveTo(const Position &) const;
			/** A list of functions to call the next time the entity moves. The functions return whether they should be removed from the queue. */
			std::list<std::function<bool(const std::shared_ptr<Entity> &)>> moveQueue;

		private:
			EntityID id_ = 0;
	};

	void to_json(nlohmann::json &, const Entity &);
}
