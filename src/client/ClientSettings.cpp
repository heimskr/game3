#include "Log.h"
#include "client/ClientSettings.h"
#include "game/ClientGame.h"
#include "ui/Window.h"
#include "util/Timer.h"

#include <boost/json.hpp>

namespace Game3 {
	void ClientSettings::apply(ClientGame &) const {
		glfwSwapInterval(capFPS? 1 : 0);
		apply();
	}

	void ClientSettings::apply() const {
		Timer::globalEnabled = !hideTimers;
		Logger::level = logLevel;
	}

	void from_json(const boost::json::value &json, ClientSettings &settings) {
		const auto &object = json.as_object();

		auto get = [&](const char *key, auto member) {
			if (auto iter = object.find(key); iter != object.end()) {
				settings.*member = boost::json::value_to<std::decay_t<decltype(settings.*member)>>(iter->value());
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
		get("specialEffects", &ClientSettings::specialEffects);
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const ClientSettings &settings) {
		auto &object = json.emplace_object();
		object["hostname"] = settings.hostname;
		object["port"] = settings.port;
		if (!settings.username.empty()) {
			object["username"] = settings.username;
		}
		object["alertOnConnection"] = settings.alertOnConnection;
		object["tickFrequency"] = settings.tickFrequency;
		object["renderLighting"] = settings.renderLighting;
		object["hideTimers"] = settings.hideTimers;
		object["logLevel"] = settings.logLevel;
		object["fpsSmoothing"] = settings.fpsSmoothing;
		object["showFPS"] = settings.showFPS;
		object["capFPS"] = settings.capFPS;
		object["specialEffects"] = settings.specialEffects;
	}
}
