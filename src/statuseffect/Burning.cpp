#include "entity/LivingEntity.h"
#include "graphics/Color.h"
#include "statuseffect/Burning.h"

namespace Game3 {
	Burning::Burning(float duration, float severity):
		StatusEffect(ID()),
		duration(duration),
		severity(severity) {}

	bool Burning::apply(const std::shared_ptr<LivingEntity> &target, float delta) {
		accumulatedDamage += delta * severity;
		float integral{};
		std::modf(accumulatedDamage, &integral);

		if (integral >= 1.0) {
			target->takeDamage(static_cast<HitPoints>(integral));
			accumulatedDamage -= integral;
		}

		duration -= delta;
		return duration <= 0;
	}

	void Burning::modifyColor(Color &color) {
		OKHsv ok = color.convert<OKHsv>();
		ok.hue = 0;
		ok.saturation = 1;
		color = ok.convert<Color>();
		color = Color{"#ff0000"};
	}
}
