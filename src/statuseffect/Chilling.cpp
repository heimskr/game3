#include "entity/LivingEntity.h"
#include "graphics/Color.h"
#include "statuseffect/Chilling.h"

namespace {
	constexpr float CHILL_SLOWING_FACTOR = 0.6;
}

namespace Game3 {
	Chilling::Chilling():
		Chilling(6) {}

	Chilling::Chilling(float duration):
		TimedStatusEffect(ID(), "base:item/bing", duration) {}

	std::string Chilling::getName() const {
		return "Chilling";
	}

	void Chilling::onAdd(const std::shared_ptr<LivingEntity> &entity) {
		entity->speedMultiplier = CHILL_SLOWING_FACTOR;
	}

	void Chilling::onRemove(const std::shared_ptr<LivingEntity> &entity) {
		entity->speedMultiplier = 1.0;
	}

	void Chilling::modifyColors(Color &, Color &composite) {
		composite = Color{"#aaaaffaa"};
	}

	std::unique_ptr<StatusEffect> Chilling::copy() const {
		return std::make_unique<Chilling>(*this);
	}
}
