#pragma once

#include "entity/Entity.h"

namespace Game3 {
	class Gene;

	class LivingEntity: public virtual Entity {
		public:
			constexpr static HitPoints INVINCIBLE = 0;
			Atomic<HitPoints> health = 0;

			/** Returns the maximum number of hitpoints this entity can have. If 0, the entity is invincible. */
			virtual HitPoints getMaxHealth() const { return 0; }
			bool isInvincible() const { return getMaxHealth() == INVINCIBLE; }

			void onSpawn() override;
			void toJSON(nlohmann::json &) const override;
			void absorbJSON(const std::shared_ptr<Game> &, const nlohmann::json &) override;
			void renderUpper(const RendererContext &) override;
			void encode(Buffer &) override;
			void decode(Buffer &) override;

			virtual bool canShowHealthBar() const;
			virtual int getDefense() const;
			virtual double getLuck() const;
			virtual HitPoints getHealth() const { return health; }
			/** Returns whether the health actually changed. */
			virtual bool setHealth(HitPoints);
			/** Returns whether the entity was healed at all. */
			virtual bool heal(HitPoints);
			/** Returns whether the entity died. */
			virtual bool takeDamage(HitPoints);
			virtual void kill();
			virtual void onAttack(const std::shared_ptr<LivingEntity> &attacker);
			virtual std::vector<ItemStackPtr> getDrops();
			virtual bool canAbsorbGenes(const nlohmann::json &) const;
			virtual void absorbGenes(const nlohmann::json &);

		protected:
			int defenseStat = 0;
			double luckStat = 0;
			/** Affects attack speed. */
			float speedStat = getBaseSpeed();

			LivingEntity();

			static float getBaseSpeed();
			static bool checkGenes(const nlohmann::json &, std::unordered_set<std::string> &&);

			template <typename T>
			inline void absorbGene(T &gene, const nlohmann::json &json, const std::string &name) {
				gene = T::fromJSON(json.at(name));
			}
	};

	using LivingEntityPtr = std::shared_ptr<LivingEntity>;
}
