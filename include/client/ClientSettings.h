#pragma once

#include "game/SimulationOptions.h"
#include "types/Types.h"

#include <boost/json/fwd.hpp>

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
		bool specialEffects = true;
		double mystery = 1;

		/** Applies settings to a game instance. */
		void apply(ClientGame &) const;

		/** Applies global settings. */
		void apply() const;
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const ClientSettings &);
	ClientSettings tag_invoke(boost::json::value_to_tag<ClientSettings>, const boost::json::value &);
}
