#include "Log.h"
#include "client/ClientSettings.h"
#include "game/ClientGame.h"
#include "ui/gtk/JSONDialog.h"
#include "ui/Canvas.h"
#include "util/Timer.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	void ClientSettings::apply(ClientGame &game) const {
		game.canvas.sizeDivisor = sizeDivisor;
		apply();
	}

	void ClientSettings::apply() const {
		Timer::globalEnabled = !hideTimers;
		Logger::level = logLevel;
	}

	std::unique_ptr<JSONDialog> ClientSettings::makeDialog(Gtk::Window &parent, std::function<void(const ClientSettings &)> submit) const {
		auto dialog = std::make_unique<JSONDialog>(parent, "Settings", nlohmann::json{
			{"hostname",          "text",   "Default Hostname",     {{"initial", hostname}}},
			{"port",              "number", "Default Port",         {{"initial", std::to_string(port)}}},
			{"username",          "text",   "Default Username",     {{"initial", username}}},
			{"alertOnConnection", "bool",   "Alert on Connection",  {{"initial", alertOnConnection}}},
			{"renderLighting",    "bool",   "Render Lighting",      {{"initial", renderLighting}}},
			{"hideTimers",        "bool",   "Hide Timer Summaries", {{"initial", hideTimers}}},
			{"logLevel",          "slider", "Log Level",            {{"range", {0,     3}}, {"increments", {1,   1}}, {"initial",      logLevel}, {"digits", 0}}},
			{"sizeDivisor",       "slider", "Size Divisor",         {{"range", {-.5,  4.}}, {"increments", {.1, .5}}, {"initial",   sizeDivisor}, {"digits", 1}}},
			{"tickFrequency",     "slider", "Tick Frequency",       {{"range", {1,   240}}, {"increments", {1,   4}}, {"initial", tickFrequency}, {"digits", 0}}},
			{"ok", "ok", "OK"},
		});

		dialog->signal_submit().connect([submit = std::move(submit)](const nlohmann::json &json) {
			submit(json.get<ClientSettings>());
		});

		return dialog;
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
