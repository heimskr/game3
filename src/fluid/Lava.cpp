#include "entity/LivingEntity.h"
#include "fluid/Lava.h"
#include "statuseffect/Burning.h"
#include "statuseffect/Chilling.h"

namespace Game3 {
	Lava::Lava(Fluid &&fluid):
		Fluid(std::move(fluid)) {}

	void Lava::onCollision(const std::shared_ptr<LivingEntity> &target) {
		if (target->getSide() == Side::Server) {
			target->inflictStatusEffect(std::make_unique<Burning>(6.0, 2.0), false);
			target->removeStatusEffect(Chilling::ID());
		}
	}
}
