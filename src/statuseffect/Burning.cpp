#include "entity/LivingEntity.h"
#include "game/Game.h"
#include "graphics/Color.h"
#include "realm/Realm.h"
#include "statuseffect/Burning.h"
#include "threading/ThreadContext.h"

namespace Game3 {
	Burning::Burning():
		Burning(6, 2) {}

	Burning::Burning(float duration, float severity):
		TimedStatusEffect(ID(), "base:item/huo", duration),
		severity(severity) {}

	std::string Burning::getName() const {
		return "Burning";
	}

	bool Burning::apply(const std::shared_ptr<LivingEntity> &target, float delta) {
		if (target->getSide() == Side::Server) {
			accumulatedDamage += delta * severity;
			float integral{};
			std::modf(accumulatedDamage, &integral);

			if (integral >= 1.0) {
				target->getGame()->enqueue([target, integral](const TickArgs &) {
					target->takeDamage(static_cast<HitPoints>(integral));
				});
				constexpr float variance = 0.95;
				target->getRealm()->playSound(target->getPosition(), "base:sound/huo", 0.2f + threadContext.random(variance, 1.f / variance));
				accumulatedDamage -= integral;
			}
		}

		return TimedStatusEffect::apply(target, delta);
	}

	void Burning::modifyColors(Color &, Color &composite) {
		composite = Color{"#ff7700aa"};
	}

	void Burning::encode(Buffer &buffer) {
		TimedStatusEffect::encode(buffer);
		buffer << severity << accumulatedDamage;
	}

	void Burning::decode(Buffer &buffer) {
		TimedStatusEffect::decode(buffer);
		buffer >> severity >> accumulatedDamage;
	}

	std::unique_ptr<StatusEffect> Burning::copy() const {
		return std::make_unique<Burning>(*this);
	}
}
