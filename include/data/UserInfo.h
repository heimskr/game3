#pragma once

#include <string>

#include <nlohmann/json.hpp>

#include "Types.h"

namespace Game3 {
	struct UserInfo {
		std::string username;
		std::string displayName;
		Token token = 0;

		UserInfo() = default;
		UserInfo(std::string_view username_, std::string_view display_name, Token token_):
			username(username_), displayName(display_name), token(token_) {}
	};

	void to_json(nlohmann::json &, const UserInfo &);
	void from_json(const nlohmann::json &, UserInfo &);
}
