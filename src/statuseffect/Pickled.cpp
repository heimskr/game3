#include "entity/LivingEntity.h"
#include "graphics/Color.h"
#include "statuseffect/Pickled.h"

namespace Game3 {
	Pickled::Pickled():
		Pickled(0) {}

	Pickled::Pickled(float duration):
		TimedStatusEffect(ID(), "base:item/pickle", duration) {}

	std::string Pickled::getName() const {
		return "Pickled";
	}

	void Pickled::modifyColors(Color &, Color &composite) {
		composite = Color{"#00aa44aa"};
	}

	std::unique_ptr<StatusEffect> Pickled::copy() const {
		return std::make_unique<Pickled>(*this);
	}
}
