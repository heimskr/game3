#pragma once

#include "config.h"

#include <functional>
#include <memory>
#include <string>

#ifndef NO_DISCORD
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
			discord::Activity activity{};
			std::function<void(discord::Result)> makeActivityCallback(std::function<void(discord::Result)> = {}) const;
			bool updateActivity(std::function<void(discord::Result)> = {});
			std::string details;
			static std::string defaultDetails();

		public:
			Discord() = default;
			~Discord();

			bool init(discord::Result *result_out = nullptr);
			bool initActivity(std::function<void(discord::Result)> callback = {});
			bool tick();
			bool setActivityDetails(const char *);
			bool setActivityDetails(std::string);
			const std::string & getDetails() const;
			void reset();
	};

	extern Discord richPresence;
}
