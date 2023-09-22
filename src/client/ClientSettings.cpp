#include "Log.h"
#include "client/ClientSettings.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	void from_json(const nlohmann::json &json, ClientSettings &settings) {
		if (auto iter = json.find("hostname"); iter != json.end())
			settings.hostname = *iter;
		if (auto iter = json.find("port"); iter != json.end())
			settings.port = *iter;
		if (auto iter = json.find("username"); iter != json.end())
			settings.username = *iter;
		if (auto iter = json.find("alertOnConnection"); iter != json.end())
			settings.alertOnConnection = *iter;
	}

	void to_json(nlohmann::json &json, const ClientSettings &settings) {
		json["hostname"] = settings.hostname;
		json["port"] = settings.port;
		if (!settings.username.empty())
			json["username"] = settings.username;
		json["alertOnConnection"] = settings.alertOnConnection;
	}
}
