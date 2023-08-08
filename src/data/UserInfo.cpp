#include "data/UserInfo.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	void to_json(nlohmann::json &json, const UserInfo &user_info) {
		json[0] = user_info.username;
		json[1] = user_info.displayName;
	}

	void from_json(const nlohmann::json &json, UserInfo &user_info) {
		user_info.username    = json.at(0);
		user_info.displayName = json.at(1);
	}
}
