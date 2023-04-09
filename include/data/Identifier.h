#pragma once

#include <stdexcept>
#include <string>

namespace Game3 {
	struct Identifier {
		std::string space;
		std::string name;

		Identifier() = default;
		Identifier(std::string space_, std::string name_): space(std::move(space_)), name(std::move(name_)) {}

		inline bool operator==(const Identifier &other) const {
			return this == &other || (space == other.space && name == other.name);
		}

		operator bool() const {
			if (space.empty() != name.empty())
				throw std::runtime_error("Partially empty identifier");

			return space.empty();
		}

		explicit inline operator std::string() const {
			return space + ':' + name;
		}

		inline bool inSpace(std::string_view check) const {
			return std::string_view(space) == check;
		}
	};
}
