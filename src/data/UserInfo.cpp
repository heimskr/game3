#include "data/UserInfo.h"

#include <boost/json.hpp>

namespace Game3 {
	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const UserInfo &user_info) {
		auto &array = json.emplace_array();
		array[0] = user_info.username;
		array[1] = user_info.displayName;
	}

	void from_json(const boost::json::value &json, UserInfo &user_info) {
		user_info.username    = std::string(json.at(0).as_string());
		user_info.displayName = std::string(json.at(1).as_string());
	}
}
