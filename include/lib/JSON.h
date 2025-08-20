#pragma once

#include "util/Concepts.h"
#include "util/Strings.h"

#include <boost/json.hpp>

#include <filesystem>
#include <format>
#include <map>
#include <ostream>
#include <unordered_map>

template <>
struct std::formatter<boost::json::value> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const boost::json::value &json, auto &ctx) const {
		boost::json::serializer serializer;
		serializer.reset(&json);
		char buf[512];

		while (!serializer.done()) {
			std::format_to(ctx.out(), "{}", static_cast<std::string_view>(serializer.read(buf)));
		}

		return ctx.out();
	}
};

template <>
struct std::formatter<boost::json::string> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const boost::json::string &json, auto &ctx) const {
		return std::format_to(ctx.out(), "{}", static_cast<std::string_view>(json));
	}
};

template <>
struct std::formatter<boost::json::object> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const boost::json::object &json, auto &ctx) const {
		boost::json::serializer serializer;
		serializer.reset(&json);
		char buf[512];

		while (!serializer.done()) {
			std::format_to(ctx.out(), "{}", static_cast<std::string_view>(serializer.read(buf)));
		}

		return ctx.out();
	}
};

namespace Game3 {
	namespace JSON = boost::json;

	template <EnumClass EC>
	void tag_invoke(JSON::value_from_tag, JSON::value &json, EC ec) {
		json = JSON::value_from(static_cast<std::underlying_type_t<EC>>(ec));
	}

	template <EnumClass EC>
	EC tag_invoke(JSON::value_to_tag<EC>, const JSON::value &json) {
		return static_cast<EC>(JSON::value_to<std::underlying_type_t<EC>>(json));
	}

	void tag_invoke(JSON::value_from_tag, JSON::value &json, const std::filesystem::path &path);
	std::filesystem::path tag_invoke(JSON::value_to_tag<std::filesystem::path>, const JSON::value &);

	JSON::object & ensureObject(JSON::value &);
	JSON::array & ensureArray(JSON::value &);
	void serializeJSON(const JSON::value &, std::ostream &);

	template <typename T>
	T getNumber(const JSON::value &json) {
		if (const double *value = json.if_double()) {
			return static_cast<T>(*value);
		}

		if (const int64_t *value = json.if_int64()) {
			return static_cast<T>(*value);
		}

		if (const uint64_t *value = json.if_uint64()) {
			return static_cast<T>(*value);
		}

		throw std::invalid_argument(std::format("Not a number: {}", json));
	}

	constexpr auto getDouble = getNumber<double>;
	constexpr auto getUint64 = getNumber<uint64_t>;
	constexpr auto getInt64 = getNumber<int64_t>;

	std::string getString(const JSON::value &);
}

namespace std {
	template <typename K, typename V, template <typename...> typename C>
	requires (std::same_as<C<K, V>, std::map<K, V>> || std::same_as<C<K, V>, std::unordered_map<K, V>>)
	auto tag_invoke(boost::json::value_to_tag<C<K, V>>, const boost::json::value &value) {
		C<K, V> out;
		for (const auto &[key, value]: value.as_object()) {
			out[Game3::fromString<K>(std::string_view(key))] = boost::json::value_to<V>(value);
		}
		return out;
	}

	template <typename K, typename V, template <typename...> typename C>
	requires (std::same_as<C<K, V>, std::map<K, V>> || std::same_as<C<K, V>, std::unordered_map<K, V>>)
	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const C<K, V> &map) {
		auto &object = json.emplace_object();
		for (const auto &[key, value]: map) {
			object[Game3::toString(key)] = boost::json::value_from(value);
		}
	}
}
