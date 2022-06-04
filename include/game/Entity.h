#pragma once

#include <string>
#include <unordered_map>
#include <utility>

#include <nlohmann/json.hpp>

#include "Types.h"
#include "game/Item.h"

namespace Game3 {
	class Entity {
		public:
			constexpr static EntityID GANGBLANC = 1;

			static std::unordered_map<EntityID, std::string> textureMap;

			EntityID id = 0;
			/** (row, column) */
			std::pair<Index, Index> position {0, 0};
			RealmID realmID = 0;
			Direction direction = Direction::Down;
			std::unordered_map<Slot, ItemStack> inventory;

			Entity(EntityID id_): id(id_) {}

			virtual bool isPlayer() const { return false; }
	};

	void to_json(nlohmann::json &, const Entity &);
	void from_json(const nlohmann::json &, Entity &);
}
