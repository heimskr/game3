#include "entity/LivingEntity.h"
#include "graphics/Color.h"
#include "statuseffect/Burning.h"

namespace Game3 {
	Burning::Burning():
		StatusEffect(ID()) {}

	Burning::Burning(float duration, float severity):
		StatusEffect(ID()),
		duration(duration),
		severity(severity) {}

	bool Burning::apply(const std::shared_ptr<LivingEntity> &target, float delta) {
		if (target->getSide() == Side::Server) {
			accumulatedDamage += delta * severity;
			float integral{};
			std::modf(accumulatedDamage, &integral);

			if (integral >= 1.0) {
				target->takeDamage(static_cast<HitPoints>(integral));
				accumulatedDamage -= integral;
			}
		}

		duration -= delta;
		return duration <= 0;
	}

	void Burning::modifyColor(Color &color) {
		color = Color{"#ff7700"};
	}

	void Burning::encode(Buffer &buffer) {
		buffer << duration << severity << accumulatedDamage;
	}

	void Burning::decode(Buffer &buffer) {
		buffer >> duration >> severity >> accumulatedDamage;
	}
}
