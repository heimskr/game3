#pragma once

#include <string>

#include <boost/json/fwd.hpp>

#include "types/Types.h"

namespace Game3 {
	struct UserInfo {
		std::string username;
		std::string displayName;

		UserInfo() = default;
		UserInfo(std::string_view username_, std::string_view display_name):
			username(username_), displayName(display_name) {}
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const UserInfo &);
	void from_json(const boost::json::value &, UserInfo &);
}
