#pragma once

#include "registry/Registerable.h"
#include "statuseffect/StatusEffect.h"
#include "types/Types.h"

#include <functional>
#include <memory>

namespace Game3 {
	class Game;

	class StatusEffectFactory: public NamedRegisterable {
		private:
			std::function<std::unique_ptr<StatusEffect>()> function;

		public:
			StatusEffectFactory(Identifier, decltype(function));

			std::unique_ptr<StatusEffect> operator()();

			template <typename T>
			static StatusEffectFactory create(Identifier identifier = T::ID()) {
				return {std::move(identifier), [] {
					return std::make_unique<T>();
				}};
			}
	};
}
