#pragma once

#include "util/Concepts.h"

#include <boost/json.hpp>

namespace Game3 {
	template <EnumClass EC>
	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, EC ec) {
		json = boost::json::value_from(static_cast<std::underlying_type_t<EC>>(ec));
	}

	template <EnumClass EC>
	EC tag_invoke(boost::json::value_to_tag<EC>, const boost::json::value &json) {
		return static_cast<EC>(boost::json::value_to<std::underlying_type_t<EC>>(json));
	}
}
