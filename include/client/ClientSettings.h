#pragma once

#include "game/SimulationOptions.h"
#include "types/Types.h"

#include <boost/json/fwd.hpp>

#include <filesystem>
#include <functional>
#include <string>

namespace Game3 {
	class ClientGame;
	class UIContext;

	struct ClientSettings {
		std::string hostname = "::1";
		std::string username;
		size_t fpsSmoothing = 200;
		Tick tickFrequency = DEFAULT_CLIENT_TICK_FREQUENCY;
		double mystery = 1;
		float uiScale = 1;
		int logLevel = 1;
		uint16_t port = 12255;
		/** If a drag action's final displacement is less than this, it will count as a click. */
		uint16_t dragThreshold = 2;
		bool alertOnConnection = false;
		bool renderLighting = false;
		bool hideTimers = true;
		bool showFPS = true;
		bool capFPS = true;
		bool specialEffects = false;
		/** Should be a relative path if it was in the current directory or any of its subdirectories, or an absolute path otherwise. */
		std::string lastWorldPath;

		ClientSettings();

		/** Applies game, UI and global settings. */
		void apply(ClientGame &) const;

		/** Applies UI and global settings. */
		void apply(UIContext &) const;

		/** Applies global settings. */
		void apply() const;

		void setLastWorldPath(const std::filesystem::path &);
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const ClientSettings &);
	ClientSettings tag_invoke(boost::json::value_to_tag<ClientSettings>, const boost::json::value &);
}
