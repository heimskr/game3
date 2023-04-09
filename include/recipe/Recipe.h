#pragma once

#include <memory>
#include <optional>

#include "registry/Registerable.h"

namespace Game3 {
	class Container;

	struct RecipeBase {
		virtual ~RecipeBase() = default;
	};

	template <typename I, typename O>
	struct Recipe: Registerable {
		using Input  = I;
		using Output = O;

		/** Returns a copy of the ingredient of the recipe. */
		virtual Input getInput() = 0;

		/** Returns a copy of the result of the recipe. */
		virtual Output getOutput() = 0;

		virtual bool canCraft(const std::shared_ptr<Container> &) = 0;

		/** Attempts to produce the result of the recipe, removing any ingredients from the given container as necessary. */
		virtual bool craft(const std::shared_ptr<Container> &, Output &leftovers) = 0;

		virtual bool operator==(const Recipe &other) {
			return this == &other;
		}
	};
}
