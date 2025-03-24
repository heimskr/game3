#include "entity/LivingEntity.h"
#include "fluid/Milk.h"
#include "statuseffect/Burning.h"

namespace Game3 {
	Milk::Milk(Fluid &&fluid):
		Fluid(std::move(fluid)) {}

	void Milk::onCollision(const std::shared_ptr<LivingEntity> &target) {
		if (target->getSide() == Side::Server) {
			target->setStatusEffects({});
		}
	}
}
