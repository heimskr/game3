#include "Log.h"
#include "client/ClientSettings.h"
#include "game/ClientGame.h"
#include "ui/Window.h"
#include "util/Timer.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	void ClientSettings::apply(ClientGame &game) const {
		game.getWindow()->sizeDivisor = sizeDivisor;
		apply();
	}

	void ClientSettings::apply() const {
		Timer::globalEnabled = !hideTimers;
		Logger::level = logLevel;
	}

	void from_json(const nlohmann::json &json, ClientSettings &settings) {
		if (auto iter = json.find("hostname"); iter != json.end())
			settings.hostname = *iter;
		if (auto iter = json.find("port"); iter != json.end())
			settings.port = *iter;
		if (auto iter = json.find("username"); iter != json.end())
			settings.username = *iter;
		if (auto iter = json.find("alertOnConnection"); iter != json.end())
			settings.alertOnConnection = *iter;
		if (auto iter = json.find("sizeDivisor"); iter != json.end())
			settings.sizeDivisor = *iter;
		if (auto iter = json.find("tickFrequency"); iter != json.end())
			settings.tickFrequency = *iter;
		if (auto iter = json.find("renderLighting"); iter != json.end())
			settings.renderLighting = *iter;
		if (auto iter = json.find("hideTimers"); iter != json.end())
			settings.hideTimers = *iter;
		if (auto iter = json.find("logLevel"); iter != json.end())
			settings.logLevel = *iter;
	}

	void to_json(nlohmann::json &json, const ClientSettings &settings) {
		json["hostname"] = settings.hostname;
		json["port"] = settings.port;
		if (!settings.username.empty())
			json["username"] = settings.username;
		json["alertOnConnection"] = settings.alertOnConnection;
		json["sizeDivisor"] = settings.sizeDivisor;
		json["tickFrequency"] = settings.tickFrequency;
		json["renderLighting"] = settings.renderLighting;
		json["hideTimers"] = settings.hideTimers;
		json["logLevel"] = settings.logLevel;
	}
}
