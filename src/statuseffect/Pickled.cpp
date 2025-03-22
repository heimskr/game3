#include "entity/LivingEntity.h"
#include "graphics/Color.h"
#include "statuseffect/Pickled.h"

namespace Game3 {
	Pickled::Pickled():
		Pickled(0) {}

	Pickled::Pickled(float duration):
		TexturedStatusEffect(ID(), "base:item/pickle"),
		duration(duration) {}

	std::string Pickled::getName() const {
		return "Pickled";
	}

	bool Pickled::apply(const std::shared_ptr<LivingEntity> &, float delta) {
		duration -= delta;
		return duration <= 0;
	}

	void Pickled::modifyColors(Color &, Color &composite) {
		composite = Color{"#00aa44aa"};
	}

	void Pickled::encode(Buffer &buffer) {
		buffer << duration;
	}

	void Pickled::decode(Buffer &buffer) {
		buffer >> duration;
	}

	std::unique_ptr<StatusEffect> Pickled::copy() const {
		return std::make_unique<Pickled>(*this);
	}
}
