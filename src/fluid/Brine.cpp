#include "entity/LivingEntity.h"
#include "fluid/Brine.h"
#include "statuseffect/Burning.h"
#include "statuseffect/Pickled.h"

namespace Game3 {
	Brine::Brine(Fluid &&fluid):
		Fluid(std::move(fluid)) {}

	void Brine::onCollision(const std::shared_ptr<LivingEntity> &target) {
		if (target->getSide() == Side::Server) {
			target->removeStatusEffect(Burning::ID());
			target->inflictStatusEffect(std::make_unique<Pickled>(6.0), false);
		}
	}
}
