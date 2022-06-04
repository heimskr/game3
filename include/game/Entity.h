#pragma once

#include <string>
#include <unordered_map>
#include <utility>

#include <nlohmann/json.hpp>

#include "Texture.h"
#include "Types.h"
#include "game/Item.h"

namespace Game3 {
	class Canvas;
	class Game;
	class Realm;
	class SpriteRenderer;

	class Entity {
		public:
			constexpr static EntityID GANGBLANC = 1;

			static std::unordered_map<EntityID, Texture> textureMap;

			/** (row, column) */
			Position position {0, 0};
			RealmID realmID = 0;
			std::weak_ptr<Realm> weakRealm;
			Direction direction = Direction::Down;
			std::unordered_map<Slot, ItemStack> inventory;

			Entity() = default;
			Entity(EntityID id__): id_(id__) {}

			static std::shared_ptr<Entity> fromJSON(const nlohmann::json &);

			virtual nlohmann::json toJSON() const;
			virtual bool isPlayer() const { return false; }
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
