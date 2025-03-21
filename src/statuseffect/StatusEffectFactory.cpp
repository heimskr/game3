#include "statuseffect/StatusEffectFactory.h"

namespace Game3 {
	StatusEffectFactory::StatusEffectFactory(Identifier identifier, decltype(function) function):
		NamedRegisterable(std::move(identifier)),
		function(std::move(function)) {}

	std::unique_ptr<StatusEffect> StatusEffectFactory::operator()() {
		if (!function) {
			throw std::logic_error("StatusEffectFactory is missing a function");
		}

		return function();
	}
}
