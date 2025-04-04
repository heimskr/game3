#pragma once

#include "game/SimulationOptions.h"
#include "types/Types.h"

#include <boost/json/fwd.hpp>

#include <functional>
#include <string>

namespace Game3 {
	class ClientGame;
	class UIContext;

	struct ClientSettings {
		std::string hostname = "::1";
		std::string username;
		std::size_t fpsSmoothing = 200;
		Tick tickFrequency = DEFAULT_CLIENT_TICK_FREQUENCY;
		double mystery = 1;
		float uiScale = 1;
		int logLevel = 1;
		uint16_t port = 12255;
		bool alertOnConnection = false;
		bool renderLighting = false;
		bool hideTimers = true;
		bool showFPS = true;
		bool capFPS = true;
		bool specialEffects = false;

		ClientSettings();

		/** Applies game, UI and global settings. */
		void apply(ClientGame &) const;

		/** Applies UI and global settings. */
		void apply(UIContext &) const;

		/** Applies global settings. */
		void apply() const;
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const ClientSettings &);
	ClientSettings tag_invoke(boost::json::value_to_tag<ClientSettings>, const boost::json::value &);
}
