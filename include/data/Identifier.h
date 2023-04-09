#pragma once

#include <stdexcept>
#include <string>

namespace Game3 {
	static constexpr bool empty(const char *str) {
		return str == nullptr || str[0] == '\0';
	}

	struct Identifier {
		const char *space = nullptr;
		const char *name = nullptr;

		constexpr Identifier() = default;
		constexpr Identifier(const char *space_, const char *name_): space(space_), name(name_) {}

		constexpr operator bool() const {
			if (empty(space) != empty(name))
				throw std::runtime_error("Partially empty identifier");

			return empty(space);
		}

		explicit inline operator std::string() const {
			return std::string(space) + ':' + std::string(name);
		}

		inline std::string str() const {
			return static_cast<std::string>(*this);
		}

		inline constexpr bool inSpace(std::string_view check) const {
			return std::string_view(space) == check;
		}

		constexpr auto operator<=>(const Identifier &) const = default;
	};
}

namespace std {
	template <>
	struct hash<Game3::Identifier> {
		size_t operator()(const Game3::Identifier &identifier) const {
			return std::hash<std::string>()(static_cast<std::string>(identifier));
		}
	};
}
