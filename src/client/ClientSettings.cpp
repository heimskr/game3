#include "client/ClientSettings.h"
#include "game/ClientGame.h"
#include "lib/JSON.h"
#include "ui/Window.h"
#include "ui/gl/Constants.h"
#include "util/FS.h"
#include "util/Log.h"
#include "util/Timer.h"

namespace Game3 {
	ClientSettings::ClientSettings():
		uiScale(UI_SCALE) {}

	void ClientSettings::apply(ClientGame &game) const {
		glfwSwapInterval(capFPS? 1 : 0);
		apply(game.getWindow()->uiContext);
	}

	void ClientSettings::apply(UIContext &ui) const {
		ui.setScale(uiScale);
		ui.dragThreshold = dragThreshold;
		apply();
	}

	void ClientSettings::apply() const {
		Timer::globalEnabled = !hideTimers;
		Logger::level = logLevel;
	}

	void ClientSettings::setLastWorldPath(const std::filesystem::path &path) {
		std::filesystem::path current = std::filesystem::current_path();
		if (isSubpath(current, path)) {
			lastWorldPath = std::filesystem::proximate(path, current);
		} else {
			lastWorldPath = std::filesystem::canonical(path);
		}
	}

	ClientSettings tag_invoke(boost::json::value_to_tag<ClientSettings>, const boost::json::value &json) {
		const auto &object = json.as_object();

		ClientSettings out;

		auto get = [&](const char *key, auto member) {
			if (auto iter = object.find(key); iter != object.end()) {
				out.*member = boost::json::value_to<std::decay_t<decltype(out.*member)>>(iter->value());
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
		get("uiScale", &ClientSettings::uiScale);
		get("dragThreshold", &ClientSettings::dragThreshold);
		get("lastWorldPath", &ClientSettings::lastWorldPath);

		return out;
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
		object["uiScale"] = settings.uiScale;
		object["dragThreshold"] = settings.dragThreshold;
		object["lastWorldPath"] = settings.lastWorldPath;
	}
}
