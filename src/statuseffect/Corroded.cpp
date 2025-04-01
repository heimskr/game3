#include "entity/LivingEntity.h"
#include "game/Game.h"
#include "graphics/Color.h"
#include "realm/Realm.h"
#include "statuseffect/Corroded.h"
#include "threading/ThreadContext.h"

namespace Game3 {
	Corroded::Corroded():
		Corroded(5, 5) {}

	Corroded::Corroded(float duration, float severity):
		TimedStatusEffect(ID(), "base:item/sulfuric_acid", duration),
		severity(severity) {}

	std::string Corroded::getName() const {
		return "Corroded";
	}

	bool Corroded::apply(const std::shared_ptr<LivingEntity> &target, float delta) {
		if (target->getSide() == Side::Server) {
			accumulatedDamage += delta * severity;
			float integral{};
			std::modf(accumulatedDamage, &integral);

			if (integral >= 1.0) {
				target->getGame()->enqueue([target, integral](const TickArgs &) {
					target->takeDamage(static_cast<HitPoints>(integral));
				});
				constexpr float variance = 0.95;
				target->getRealm()->playSound(target->getPosition(), "base:sound/burn", 0.2f + threadContext.random(variance, 1.f / variance));
				severity /= 1.05;
				accumulatedDamage -= integral;
			}
		}

		return TimedStatusEffect::apply(target, delta);
	}

	void Corroded::modifyColors(Color &, Color &composite) {
		composite = Color{"#ccff33aa"};
	}

	void Corroded::encode(Buffer &buffer) {
		TimedStatusEffect::encode(buffer);
		buffer << severity << accumulatedDamage;
	}

	void Corroded::decode(Buffer &buffer) {
		TimedStatusEffect::decode(buffer);
		buffer >> severity >> accumulatedDamage;
	}

	std::unique_ptr<StatusEffect> Corroded::copy() const {
		return std::make_unique<Corroded>(*this);
	}
}
