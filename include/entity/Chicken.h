#pragma once

#include "entity/Animal.h"

namespace Game3 {
	class Building;

	class Chicken: public Animal {
		public:
			static constexpr float EGG_PERIOD = 60.f;

			float timeUntilEgg = EGG_PERIOD;

			static Identifier ID() { return {"base", "entity/chicken"}; }

			static std::shared_ptr<Chicken> create(Game &) {
				return Entity::create<Chicken>();
			}

			static std::shared_ptr<Chicken> fromJSON(Game &game, const nlohmann::json &json) {
				auto out = Entity::create<Chicken>();
				out->absorbJSON(game, json);
				return out;
			}

			std::string getName() const override { return "Chicken"; }

			void tick(Game &, float) override;
			void encode(Buffer &) override;
			void decode(Buffer &) override;

		protected:
			Chicken(): Animal(ID()) {}

		friend class Entity;
	};
}
