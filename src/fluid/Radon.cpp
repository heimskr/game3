#include "entity/LivingEntity.h"
#include "fluid/Radon.h"
#include "statuseffect/Irradiated.h"

namespace Game3 {
	Radon::Radon(Fluid &&fluid):
		Fluid(std::move(fluid)) {}

	void Radon::onCollision(const std::shared_ptr<LivingEntity> &target) {
		if (target->getSide() == Side::Server) {
			target->inflictStatusEffect(std::make_unique<Irradiated>(), false);
		}
	}
}
