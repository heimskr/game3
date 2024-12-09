#pragma once

#include "util/Concepts.h"

#include <boost/json.hpp>

#include <filesystem>
#include <format>

namespace Game3 {
	template <EnumClass EC>
	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, EC ec) {
		json = boost::json::value_from(static_cast<std::underlying_type_t<EC>>(ec));
	}

	template <EnumClass EC>
	EC tag_invoke(boost::json::value_to_tag<EC>, const boost::json::value &json) {
		return static_cast<EC>(boost::json::value_to<std::underlying_type_t<EC>>(json));
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const std::filesystem::path &path);
	std::filesystem::path tag_invoke(boost::json::value_to_tag<std::filesystem::path>, const boost::json::value &);
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
