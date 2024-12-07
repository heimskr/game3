#pragma once

#include "registry/Registerable.h"

#include <boost/json/fwd.hpp>

#include <memory>
#include <optional>

namespace Game3 {
	class Game;
	struct Container;

	template <typename I, typename O, typename R = Registerable>
	struct Recipe: R {
		using Input  = I;
		using Output = O;

		using R::R;

		Recipe(const Recipe &) = default;
		Recipe(Recipe &&) noexcept = default;

		virtual ~Recipe() = default;

		Recipe & operator=(const Recipe &) = default;
		Recipe & operator=(Recipe &&) noexcept = default;

		/** Returns a copy of the ingredient of the recipe. */
		virtual Input getInput(const std::shared_ptr<Game> &) = 0;

		/** Returns a copy of the result of the recipe for a given input. */
		virtual Output getOutput(const Input &, const std::shared_ptr<Game> &) = 0;

		virtual bool canCraft(const std::shared_ptr<Container> &) = 0;

		/** Attempts to produce the result of the recipe, removing any ingredients from the given container as necessary. May or may not fail (depending on the recipe type) if there would be any leftover output. */
		virtual bool craft(const std::shared_ptr<Game> &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftovers) = 0;

		/** Attempts to produce the result of the recipe, removing any ingredients from the given container as necessary. Fails without making any changes if there would be any leftover output. */
		virtual bool craft(const std::shared_ptr<Game> &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container) = 0;

		virtual void toJSON(boost::json::value &) const = 0;

		virtual bool operator==(const Recipe &other) {
			return this == &other;
		}
	};
}
