#pragma once

#include <cstddef>

#include "data/Identifier.h"

namespace Game3 {
	struct Registerable {
		size_t registryID = -1;
		protected:
			Registerable() = default;
	};

	struct NamedRegisterable: Registerable {
		Identifier identifier;

		NamedRegisterable() = delete;
		NamedRegisterable(Identifier identifier):
			identifier(std::move(identifier)) {}
	};

	struct NumericRegisterable: Registerable {
		using Type = size_t;

		Type number;

		NumericRegisterable() = delete;
		NumericRegisterable(Type number):
			number(number) {}
	};

	struct StringRegisterable: Registerable {
		using Type = std::string;

		Type name;

		StringRegisterable() = delete;
		StringRegisterable(Type name):
			name(std::move(name)) {}
	};
}
