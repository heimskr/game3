#pragma once

#include <format>
#include <ostream>
#include <stdexcept>
#include <string>

#include <boost/json/fwd.hpp>

namespace Game3 {
	class Buffer;

	struct Identifier {
		std::string space;
		std::string name;

		constexpr Identifier() = default;
		constexpr Identifier(const char *space_, const char *name_): space(space_), name(name_) {}
		constexpr Identifier(std::string space_, std::string name_): space(std::move(space_)), name(std::move(name_)) {}

		constexpr Identifier(std::string_view combined) {
			const size_t colon = combined.find(':');
			if (colon == std::string_view::npos)
				throw std::invalid_argument("Not a valid identifier: " + std::string(combined));
			space = std::string(combined.substr(0, colon));
			name  = std::string(combined.substr(colon + 1));
		}

		constexpr Identifier(const char *combined):
			Identifier(std::string_view(combined)) {}

		inline explicit constexpr operator bool() const {
			return !empty();
		}

		inline constexpr bool empty() const {
			if (space.empty() != name.empty())
				throw std::runtime_error("Partially empty identifier");
			return space.empty();
		}

		inline explicit constexpr operator std::string() const {
			return space + ':' + name;
		}

		inline constexpr std::string str() const {
			return static_cast<std::string>(*this);
		}

		inline constexpr bool inSpace(std::string_view check) const {
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

	Identifier tag_invoke(boost::json::value_to_tag<Identifier>, const boost::json::value &);
	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const Identifier &);

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
		std::hash<std::string> hasher;
		uintmax_t hash = hasher(identifier.space);
		hash <<= sizeof(uintmax_t) * 4;
		hash ^= hasher(identifier.name);
		return std::hash<uintmax_t>{}(hash);
	}
};

template <>
struct std::formatter<Game3::Identifier> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &identifier, auto &ctx) const {
		return std::format_to(ctx.out(), "{}:{}", identifier.space, identifier.name);
	}
};
