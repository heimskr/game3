#pragma once

#include <string>
#include <unordered_map>
#include <utility>

#include <nanogui/common.h>
#include <nlohmann/json.hpp>

#include "Texture.h"
#include "Types.h"
#include "game/Inventory.h"
#include "game/Item.h"

namespace Game3 {
	class Canvas;
	class Game;
	class Realm;
	class SpriteRenderer;

	class Entity {
		public:
			constexpr static EntityID GANGBLANC = 1;
			constexpr static EntityID GRUM = 2;

			static std::unordered_map<EntityID, Texture> textureMap;

			/** (row, column) */
			Position position {0, 0};
			RealmID realmID = 0;
			std::weak_ptr<Realm> weakRealm;
			Direction direction = Direction::Down;
			Inventory inventory {16};
			/** The reciprocal of this is how many seconds it takes to move one square. */
			float speed = 10.f;
			/** When the entity moves a square, its position field is immediately updated but this field is set to an offset
			 *  such that the sum of the new position and the offset is equal to the old offset. The offset is moved closer
			 *  to zero each tick to achieve smooth movement instead of teleportation from one tile to the next. */
			nanogui::Vector2f offset {0.f, 0.f};

			Entity() = default;
			Entity(EntityID id__): id_(id__) {}

			static std::shared_ptr<Entity> fromJSON(const nlohmann::json &);

			virtual nlohmann::json toJSON() const;
			virtual bool isPlayer() const { return false; }
			virtual void tick(float delta);
			inline EntityID id() const { return id_; }
			inline const Position::first_type  & row()    const { return position.first;  }
			inline const Position::second_type & column() const { return position.second; }
			inline Position::first_type  & row()    { return position.first;  }
			inline Position::second_type & column() { return position.second; }
			void id(EntityID);
			void init();
			void render(SpriteRenderer &) const;
			void move(Direction);
			void setRealm(const Game &, RealmID);
			void setRealm(const std::shared_ptr<Realm>);
			void focus(Canvas &);

		private:
			EntityID id_ = 0;
			Texture *texture = nullptr;

			bool canMoveTo(const Position &) const;
	};

	void to_json(nlohmann::json &, const Entity &);
	void from_json(const nlohmann::json &, Entity &);
}
