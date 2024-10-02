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

	struct ClientSettings {
		std::string hostname = "::1";
		uint16_t port = 12255;
		std::string username;
		bool alertOnConnection = true;
		Tick tickFrequency = DEFAULT_CLIENT_TICK_FREQUENCY;
		bool renderLighting = false;
		bool hideTimers = true;
		int logLevel = 1;
		std::size_t fpsSmoothing = 200;
		bool showFPS = true;
		bool capFPS = true;

		/** Applies settings to a game instance. */
		void apply(ClientGame &) const;

		/** Applies global settings. */
		void apply() const;
	};

	void from_json(const nlohmann::json &, ClientSettings &);
	void to_json(nlohmann::json &, const ClientSettings &);
}
