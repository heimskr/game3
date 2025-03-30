#include "entity/LivingEntity.h"
#include "fluid/SulfuricAcid.h"
#include "statuseffect/Corroded.h"

namespace Game3 {
	SulfuricAcid::SulfuricAcid(Fluid &&fluid):
		Fluid(std::move(fluid)) {}

	void SulfuricAcid::onCollision(const std::shared_ptr<LivingEntity> &target) {
		if (target->getSide() == Side::Server) {
			target->inflictStatusEffect(std::make_unique<Corroded>(), false);
		}
	}
}
