#pragma once

#include "entity/Monster.h"
#include "item/Item.h"

namespace Game3 {
	class Building;

	class Eye: public Monster {
		public:
			static Identifier ID() { return {"base", "entity/eye"}; }

			static std::shared_ptr<Eye> create(const std::shared_ptr<Game> &) {
				return Entity::create<Eye>();
			}

			static std::shared_ptr<Eye> fromJSON(const std::shared_ptr<Game> &game, const nlohmann::json &json) {
				auto out = Entity::create<Eye>();
				out->absorbJSON(game, json);
				return out;
			}

			std::string getName() const override { return "Eye"; }
			float getMovementSpeed() const override { return 7.f; }

			HitPoints getBaseDamage() const override { return 2; }
			int getVariability() const override { return 1; }
			float getAttackPeriod() const override { return 1.f; }
			std::vector<ItemStackPtr> getDrops() override {
				std::vector<ItemStackPtr> out = Monster::getDrops();
				out.push_back(ItemStack::create(getGame(), "base:item/eye"));
				return out;
			}

		protected:
			Eye():
				Entity(ID()), Monster() { luckStat = 0.5; }

		friend class Entity;
	};
}
