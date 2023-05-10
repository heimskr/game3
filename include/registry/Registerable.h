#pragma once

#include <cstddef>

#include "data/Identifier.h"

namespace Game3 {
	struct Registerable {
		size_t registryID = -1;
	};

	struct NamedRegisterable: Registerable {
		Identifier identifier;

		NamedRegisterable() = delete;
		NamedRegisterable(Identifier identifier_): identifier(std::move(identifier_)) {}
	};

	struct NumericRegisterable: Registerable {
		using Type = size_t;

		Type number;

		NumericRegisterable() = delete;
		NumericRegisterable(Type number_): number(number_) {}
	};
}
