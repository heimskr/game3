#include "Log.h"
#include "client/ClientSettings.h"
#include "game/ClientGame.h"
#include "ui/Window.h"
#include "util/Timer.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	void ClientSettings::apply(ClientGame &) const {
		glfwSwapInterval(capFPS? 1 : 0);
		apply();
	}

	void ClientSettings::apply() const {
		Timer::globalEnabled = !hideTimers;
		Logger::level = logLevel;
	}

	void from_json(const nlohmann::json &json, ClientSettings &settings) {
		auto get = [&](const char *key, auto member) {
			if (auto iter = json.find(key); iter != json.end()) {
				settings.*member = *iter;
			}
		};

		get("hostname", &ClientSettings::hostname);
		get("port", &ClientSettings::port);
		get("username", &ClientSettings::username);
		get("alertOnConnection", &ClientSettings::alertOnConnection);
		get("tickFrequency", &ClientSettings::tickFrequency);
		get("renderLighting", &ClientSettings::renderLighting);
		get("hideTimers", &ClientSettings::hideTimers);
		get("logLevel", &ClientSettings::logLevel);
		get("fpsSmoothing", &ClientSettings::fpsSmoothing);
		get("showFPS", &ClientSettings::showFPS);
		get("capFPS", &ClientSettings::capFPS);
	}

	void to_json(nlohmann::json &json, const ClientSettings &settings) {
		json["hostname"] = settings.hostname;
		json["port"] = settings.port;
		if (!settings.username.empty())
			json["username"] = settings.username;
		json["alertOnConnection"] = settings.alertOnConnection;
		json["tickFrequency"] = settings.tickFrequency;
		json["renderLighting"] = settings.renderLighting;
		json["hideTimers"] = settings.hideTimers;
		json["logLevel"] = settings.logLevel;
		json["fpsSmoothing"] = settings.fpsSmoothing;
		json["showFPS"] = settings.showFPS;
		json["capFPS"] = settings.capFPS;
	}
}
