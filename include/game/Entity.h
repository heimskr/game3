#pragma once

#include <string>
#include <unordered_map>
#include <utility>

#include <nlohmann/json.hpp>

#include "Texture.h"
#include "Types.h"
#include "game/Item.h"

namespace Game3 {
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

			Entity(EntityID id__): id_(id__) {}

			virtual bool isPlayer() const { return false; }
			EntityID id() const { return id_; }
			void id(EntityID);
			void init();
			void render(SpriteRenderer &) const;
			void move(Direction);
			void setRealm(const Game &, RealmID);
			void setRealm(const std::shared_ptr<Realm>);

		private:
			EntityID id_ = 0;
			Texture *texture = nullptr;

			bool canMoveTo(const Position &) const;
	};

	void to_json(nlohmann::json &, const Entity &);
	void from_json(const nlohmann::json &, Entity &);
}
