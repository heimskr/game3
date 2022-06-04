#pragma once

#include "game/Entity.h"

namespace Game3 {
	class Player: public Entity {
		public:
			using Entity::Entity;

			bool isPlayer() const override { return true; }
	};

	void to_json(nlohmann::json &, const Player &);
	void from_json(const nlohmann::json &, Player &);
}
