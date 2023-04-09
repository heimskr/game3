#pragma once

#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

namespace Game3 {

	struct Identifier {
		std::string space;
		std::string name;

		Identifier() = default;
		Identifier(const char *space_, const char *name_): space(space_), name(name_) {}
		Identifier(std::string_view);
		Identifier(const char *);

		operator bool() const {
			if (space.empty() != name.empty())
				throw std::runtime_error("Partially empty identifier");

			return space.empty();
		}

		explicit inline operator std::string() const {
			return space + ':' + name;
		}

		inline std::string str() const {
			return static_cast<std::string>(*this);
		}

		inline bool inSpace(std::string_view check) const {
			return std::string_view(space) == check;
		}

		/** Returns "foo/bar" for "base:foo/bar/baz". */
		std::string getPath() const;

		/** Returns "foo" for "base:foo/bar/baz". */
		std::string getPathStart() const;

		auto operator<=>(const Identifier &) const = default;

		bool operator==(std::string_view) const;
	};

	void from_json(const nlohmann::json &, Identifier &);
	void to_json(nlohmann::json &, const Identifier &);
}

namespace std {
	template <>
	struct hash<Game3::Identifier> {
		size_t operator()(const Game3::Identifier &identifier) const {
			return std::hash<std::string>()(static_cast<std::string>(identifier));
		}
	};
}
