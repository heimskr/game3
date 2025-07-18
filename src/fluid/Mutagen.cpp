#include "entity/LivingEntity.h"
#include "fluid/Mutagen.h"
#include "statuseffect/Burning.h"
#include "statuseffect/Chilling.h"

namespace Game3 {
	Mutagen::Mutagen(Fluid &&fluid):
		Fluid(std::move(fluid)) {}

	void Mutagen::onCollision(const std::shared_ptr<LivingEntity> &target) {
		if (target->getSide() == Side::Server) {
			target->mutate(1);
		}
	}
}
