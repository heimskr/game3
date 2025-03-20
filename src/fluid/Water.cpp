#include "entity/LivingEntity.h"
#include "fluid/Water.h"
#include "statuseffect/Burning.h"

namespace Game3 {
	Water::Water(Fluid &&fluid):
		Fluid(std::move(fluid)) {}

	void Water::onCollision(const std::shared_ptr<LivingEntity> &target) {
		if (target->getSide() == Side::Server) {
			target->removeStatusEffect(Burning::ID());
		}
	}
}
