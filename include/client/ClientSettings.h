#pragma once

#include <nlohmann/json_fwd.hpp>
#include <string>

namespace Game3 {
	struct ClientSettings {
		std::string hostname = "::1";
		uint16_t port = 12255;
		std::string username;
		bool alertOnConnection = true;
	};

	void from_json(const nlohmann::json &, ClientSettings &);
	void to_json(nlohmann::json &, const ClientSettings &);
}
