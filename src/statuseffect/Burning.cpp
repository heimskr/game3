#include "entity/LivingEntity.h"
#include "graphics/Color.h"
#include "statuseffect/Burning.h"

namespace Game3 {
	Burning::Burning():
		Burning(0, 0) {}

	Burning::Burning(float duration, float severity):
		TexturedStatusEffect(ID(), "base:item/fire"),
		duration(duration),
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
				target->takeDamage(static_cast<HitPoints>(integral));
				accumulatedDamage -= integral;
			}
		}

		duration -= delta;
		return duration <= 0;
	}

	void Burning::modifyColors(Color &, Color &composite) {
		composite = Color{"#ff7700aa"};
	}

	void Burning::encode(Buffer &buffer) {
		buffer << duration << severity << accumulatedDamage;
	}

	void Burning::decode(Buffer &buffer) {
		buffer >> duration >> severity >> accumulatedDamage;
	}

	std::unique_ptr<StatusEffect> Burning::copy() const {
		return std::make_unique<Burning>(*this);
	}
}
