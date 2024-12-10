#pragma once

#include "util/Concepts.h"

#include <boost/json.hpp>

#include <filesystem>
#include <format>
#include <ostream>

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
	void serializeJSON(const JSON::value &, std::ostream &);
}

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
