#include "entity/LivingEntity.h"
#include "game/Game.h"
#include "graphics/Color.h"
#include "statuseffect/Irradiated.h"

namespace Game3 {
	Irradiated::Irradiated():
		Irradiated(5, 2) {}

	Irradiated::Irradiated(float duration, float severity):
		TimedStatusEffect(ID(), "base:item/you", duration),
		severity(severity) {}

	std::string Irradiated::getName() const {
		return "Irradiated";
	}

	bool Irradiated::apply(const std::shared_ptr<LivingEntity> &target, float delta) {
		if (target->getSide() == Side::Server) {
			accumulatedDamage += delta * severity;
			float integral{};
			std::modf(accumulatedDamage, &integral);

			if (integral >= 1.0) {
				target->enqueueDamage(static_cast<HitPoints>(integral));
				severity *= 1.05;
				accumulatedDamage -= integral;
			}
		}

		return TimedStatusEffect::apply(target, delta);
	}

	void Irradiated::modifyColors(Color &, Color &composite) {
		composite = Color{"#77ff00aa"};
	}

	void Irradiated::encode(Buffer &buffer) {
		TimedStatusEffect::encode(buffer);
		buffer << severity << accumulatedDamage;
	}

	void Irradiated::decode(Buffer &buffer) {
		TimedStatusEffect::decode(buffer);
		buffer >> severity >> accumulatedDamage;
	}

	std::unique_ptr<StatusEffect> Irradiated::copy() const {
		return std::make_unique<Irradiated>(*this);
	}
}
