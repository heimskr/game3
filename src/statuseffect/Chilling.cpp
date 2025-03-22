#include "entity/LivingEntity.h"
#include "graphics/Color.h"
#include "statuseffect/Chilling.h"

namespace {
	constexpr float CHILL_SLOWING_FACTOR = 0.6;
}

namespace Game3 {
	Chilling::Chilling():
		Chilling(0) {}

	Chilling::Chilling(float duration):
		TexturedStatusEffect(ID(), "base:item/bing"),
		duration(duration) {}

	std::string Chilling::getName() const {
		return "Chilling";
	}

	bool Chilling::apply(const std::shared_ptr<LivingEntity> &, float delta) {
		duration -= delta;
		return duration <= 0;
	}

	void Chilling::onAdd(const std::shared_ptr<LivingEntity> &entity) {
		if (const float old = entity->baseSpeed; old > 1.0) {
			wasAdded = true;
			entity->baseSpeed = old * CHILL_SLOWING_FACTOR;
		} else {
			wasAdded = false;
		}
	}

	void Chilling::onRemove(const std::shared_ptr<LivingEntity> &entity) {
		if (wasAdded) {
			entity->baseSpeed = entity->baseSpeed / CHILL_SLOWING_FACTOR;
		}
	}

	void Chilling::modifyColors(Color &, Color &composite) {
		composite = Color{"#aaaaffaa"};
	}

	void Chilling::encode(Buffer &buffer) {
		buffer << duration;
	}

	void Chilling::decode(Buffer &buffer) {
		buffer >> duration;
	}

	std::unique_ptr<StatusEffect> Chilling::copy() const {
		return std::make_unique<Chilling>(*this);
	}
}
