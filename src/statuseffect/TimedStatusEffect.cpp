#include "game/ClientGame.h"
#include "statuseffect/TimedStatusEffect.h"

namespace Game3 {
	TimedStatusEffect::TimedStatusEffect(Identifier identifier, Identifier itemID, float duration):
		TexturedStatusEffect(std::move(identifier), std::move(itemID)),
		duration(duration),
		originalDuration(duration) {}

	bool TimedStatusEffect::apply(const std::shared_ptr<LivingEntity> &, float delta) {
		duration -= delta;
		return duration <= 0;
	}

	void TimedStatusEffect::replenish(const std::shared_ptr<LivingEntity> &) {
		duration = originalDuration;
	}

	void TimedStatusEffect::encode(Buffer &buffer) {
		buffer << originalDuration << duration;
	}

	void TimedStatusEffect::decode(BasicBuffer &buffer) {
		buffer >> originalDuration >> duration;
	}
}
