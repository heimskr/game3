#pragma once

#include "config.h"

#include <chrono>
#include <functional>
#include <memory>
#include <string>

#ifdef DISCORD_RICH_PRESENCE
#include "discord.h"
#else
namespace discord {
	struct Core {};
	struct Activity {};
	enum class Result {};
}
#endif

namespace Game3 {
	class Discord {
		private:
			std::unique_ptr<discord::Core> core;
#ifdef DISCORD_RICH_PRESENCE
			discord::Activity activity{};
#endif
			std::string details;
			std::chrono::system_clock::time_point startingTime;

			std::function<void(discord::Result)> makeActivityCallback(std::function<void(discord::Result)> = {}) const;
			bool updateActivity(std::function<void(discord::Result)> = {});
			static std::string defaultDetails();

		public:
			Discord() = default;
			~Discord();

			bool init(discord::Result *result_out = nullptr);
			bool initActivity(std::function<void(discord::Result)> callback = {});
			bool tick();
			bool setActivityDetails(const char *, bool update = true);
			bool setActivityDetails(std::string, bool update = true);
			bool setActivityStartTime(std::chrono::system_clock::time_point = std::chrono::system_clock::now(), bool update = true);
			bool setActivityStartTime(bool update = true);
			const std::string & getDetails() const;
			void reset();
	};

	extern Discord richPresence;
}
