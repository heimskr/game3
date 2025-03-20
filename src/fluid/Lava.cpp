#include "entity/LivingEntity.h"
#include "fluid/Lava.h"
#include "statuseffect/Burning.h"

namespace Game3 {
	Lava::Lava(Fluid &&fluid):
		Fluid(std::move(fluid)) {}

	void Lava::onCollision(const std::shared_ptr<LivingEntity> &target) {
		target->inflictStatusEffect(std::make_unique<Burning>(4.0, 2.0), true);
	}
}
