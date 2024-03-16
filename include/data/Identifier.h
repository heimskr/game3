#pragma once

#include <format>
#include <ostream>
#include <stdexcept>
#include <string>

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class Buffer;

	struct Identifier {
		std::string space;
		std::string name;

		Identifier() = default;
		Identifier(const char *space_, const char *name_): space(space_), name(name_) {}
		Identifier(std::string space_, std::string name_): space(std::move(space_)), name(std::move(name_)) {}
		Identifier(std::string_view);
		Identifier(const char *);

		inline explicit operator bool() const {
			return !empty();
		}

		inline bool empty() const {
			if (space.empty() != name.empty())
				throw std::runtime_error("Partially empty identifier");
			return space.empty();
		}

		inline explicit operator std::string() const {
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

		/** Returns "baz" for "base:foo/bar/baz". */
		std::string getPostPath() const;

		bool operator==(const char *) const;
		bool operator==(std::string_view) const;
		bool operator==(const Identifier &) const;
		bool operator<(const Identifier &) const;
	};

	std::ostream & operator<<(std::ostream &, const Identifier &);

	void from_json(const nlohmann::json &, Identifier &);
	void to_json(nlohmann::json &, const Identifier &);

	Identifier operator""_id(const char *string, size_t);
	template <typename T>
	T popBuffer(Buffer &);
	template <>
	Identifier popBuffer<Identifier>(Buffer &);
	Buffer & operator+=(Buffer &, const Identifier &);
	Buffer & operator<<(Buffer &, const Identifier &);
	Buffer & operator>>(Buffer &, Identifier &);
}

template <>
struct std::hash<Game3::Identifier> {
	size_t operator()(const Game3::Identifier &identifier) const {
		return std::hash<std::string>()(static_cast<std::string>(identifier));
	}
};

template <>
struct std::formatter<Game3::Identifier> {
	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(const auto &identifier, std::format_context &ctx) const {
		return std::format_to(ctx.out(), "{}:{}", identifier.space, identifier.name);
	}
};
