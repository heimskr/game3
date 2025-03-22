#include "entity/LivingEntity.h"
#include "graphics/Color.h"
#include "statuseffect/Chilling.h"

namespace Game3 {
	Chilling::Chilling():
		Chilling(0) {}

	Chilling::Chilling(float duration):
		TexturedStatusEffect(ID(), "base:item/snowball"),
		duration(duration) {}

	std::string Chilling::getName() const {
		return "Chilling";
	}

	bool Chilling::apply(const std::shared_ptr<LivingEntity> &, float delta) {
		duration -= delta;
		return duration <= 0;
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
