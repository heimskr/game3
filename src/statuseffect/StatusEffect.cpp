#include "statuseffect/StatusEffect.h"

namespace Game3 {
	StatusEffect::StatusEffect(Identifier identifier):
		NamedRegisterable(std::move(identifier)) {}

	StatusEffect::~StatusEffect() = default;
}
