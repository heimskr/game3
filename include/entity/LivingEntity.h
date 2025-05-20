#pragma once

#include "entity/Entity.h"
#include "statuseffect/StatusEffectMap.h"

#include <functional>

namespace Game3 {
	class Gene;
	class StatusEffect;

	class LivingEntity: public virtual Entity {
		public:
			constexpr static HitPoints INVINCIBLE = 0;
			Atomic<HitPoints> health = 0;

			/** Returns the maximum number of hitpoints this entity can have. If 0, the entity is invincible. */
			virtual HitPoints getMaxHealth() const { return 0; }
			bool isInvincible() const { return getMaxHealth() == INVINCIBLE; }

			void tick(const TickArgs &) override;
			void onSpawn() override;
			void toJSON(boost::json::value &) const override;
			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;
			void renderUpper(const RendererContext &) override;
			void encode(Buffer &) override;
			void decode(Buffer &) override;
			bool isAffectedByKnockback() const override;
			std::pair<Color, Color> getColors() const override;
			bool setField(uint32_t field_name, Buffer &field_value, const PlayerPtr &updater) override;

			virtual bool canShowHealthBar() const;
			virtual int getDefense() const;
			virtual double getLuck() const;
			virtual bool atFullHealth() const;
			virtual HitPoints getHealth() const { return health; }
			/** Returns whether the health actually changed. */
			virtual bool setHealth(HitPoints);
			/** Returns whether the entity was healed at all. */
			virtual bool heal(HitPoints);
			/** Returns whether the entity died. */
			virtual bool takeDamage(HitPoints);
			/** Returns whether the entity died. `angle` is in radians. */
			virtual bool hitByExplosion(float damage_scale, float radius, float distance, float angle);
			virtual void enqueueDamage(HitPoints);
			virtual void spawnBlood(size_t count);
			virtual void kill();
			virtual void onAttack(const std::shared_ptr<LivingEntity> &attacker);
			virtual std::vector<ItemStackPtr> getDrops();
			virtual bool canAbsorbGenes(const boost::json::value &) const;
			virtual void absorbGenes(const boost::json::value &);
			virtual void iterateGenes(const std::function<void(Gene &)> &);
			virtual void iterateGenes(const std::function<void(const Gene &)> &) const;
			virtual void inflictStatusEffect(std::unique_ptr<StatusEffect> &&, bool can_overwrite);
			virtual void removeStatusEffect(const Identifier &);
			virtual void setStatusEffects(StatusEffectMap);
			virtual StatusEffectMap copyStatusEffects() const;
			/** Returns whether iteration was stopped by the supplied function returning true. */
			virtual bool iterateStatusEffects(const std::function<bool(const Identifier &, const std::unique_ptr<StatusEffect> &)> &) const;
			virtual bool susceptibleToStatusEffect(const Identifier &) const;

			std::shared_ptr<LivingEntity> getSelf();
			std::weak_ptr<LivingEntity> getWeakSelf();

		protected:
			Lockable<StatusEffectMap> statusEffects;
			double luckStat = 0;
			/** Affects attack speed. */
			float speedStat = getBaseSpeed();
			int defenseStat = 0;
			/** Not synchronized. */
			int kills = 0;

			LivingEntity();

			static float getBaseSpeed();
			static bool checkGenes(const boost::json::value &, std::unordered_set<std::string> &&);

			template <typename T>
			inline void absorbGene(T &gene, const boost::json::value &json, const std::string &name) {
				gene = T::fromJSON(json.at(name));
			}
	};

	using LivingEntityPtr = std::shared_ptr<LivingEntity>;
}
