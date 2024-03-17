#pragma once

#include "game/SimulationOptions.h"
#include "types/Types.h"

#include <nlohmann/json_fwd.hpp>

#include <functional>
#include <string>

namespace Gtk {
	class Window;
}

namespace Game3 {
	class ClientGame;
	class JSONDialog;

	struct ClientSettings {
		std::string hostname = "::1";
		uint16_t port = 12255;
		std::string username;
		bool alertOnConnection = true;
		double sizeDivisor = 1.0;
		Tick tickFrequency = DEFAULT_CLIENT_TICK_FREQUENCY;
		bool renderLighting = true;
		bool hideTimers = true;
		int logLevel = 1;

		/** Applies settings to a game instance. */
		void apply(ClientGame &) const;

		/** Applies global settings. */
		void apply() const;

		std::unique_ptr<JSONDialog> makeDialog(Gtk::Window &parent, std::function<void(const ClientSettings &)> submit) const;
	};

	void from_json(const nlohmann::json &, ClientSettings &);
	void to_json(nlohmann::json &, const ClientSettings &);
}
