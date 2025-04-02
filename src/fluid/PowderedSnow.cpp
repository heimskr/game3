#include "entity/LivingEntity.h"
#include "fluid/PowderedSnow.h"
#include "statuseffect/Burning.h"
#include "statuseffect/Chilling.h"

namespace Game3 {
	PowderedSnow::PowderedSnow(Fluid &&fluid):
		Fluid(std::move(fluid)) {}

	void PowderedSnow::onCollision(const std::shared_ptr<LivingEntity> &target) {
		if (target->getSide() == Side::Server) {
			target->removeStatusEffect(Burning::ID());
			target->inflictStatusEffect(std::make_unique<Chilling>(), false);
		}
	}
}
