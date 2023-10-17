#pragma once

#include <string>

#include <nlohmann/json_fwd.hpp>

#include "types/Types.h"

namespace Game3 {
	struct UserInfo {
		std::string username;
		std::string displayName;

		UserInfo() = default;
		UserInfo(std::string_view username_, std::string_view display_name):
			username(username_), displayName(display_name) {}
	};

	void to_json(nlohmann::json &, const UserInfo &);
	void from_json(const nlohmann::json &, UserInfo &);
}
