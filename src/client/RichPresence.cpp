#include "client/RichPresence.h"
#include "util/Log.h"

namespace Game3 {
	Discord::~Discord() {
		reset();
	}

	bool Discord::init(discord::Result *result_out) {
#ifndef DISCORD_RICH_PRESENCE
		(void) result_out;
		return false;
#else
		discord::Core *core_raw{};
		const discord::Result result = discord::Core::Create(1155446965744709672, DiscordCreateFlags_NoRequireDiscord, &core_raw);

		if (result_out != nullptr) {
			*result_out = result;
		}

		if (result != discord::Result::Ok) {
			ERR("Couldn't initialize Discord: {}", int(result));
			return false;
		}

		core.reset(core_raw);
		return true;
#endif
	}

	std::function<void(discord::Result)> Discord::makeActivityCallback(std::function<void(discord::Result)> callback) const {
#ifdef DISCORD_RICH_PRESENCE
		return [callback = std::move(callback)](discord::Result result) {
			if (result != discord::Result::Ok) {
				if (result == discord::Result::TransactionAborted) {
					ERR(3, "Couldn't set activity: transaction aborted");
				} else {
					ERR("Couldn't set activity: {}", int(result));
				}
			}
			if (callback) {
				callback(result);
			}
		};
#else
		(void) callback;
		return {};
#endif
	}

	bool Discord::updateActivity(std::function<void(discord::Result)> callback) {
#ifdef DISCORD_RICH_PRESENCE
		if (!core) {
			return false;
		}

		core->ActivityManager().UpdateActivity(activity, makeActivityCallback(std::move(callback)));
		return true;
#else
		(void) callback;
		return false;
#endif
	}

	bool Discord::initActivity(std::function<void(discord::Result)> callback) {
		if (!core) {
			return false;
		}

		details = defaultDetails();
#ifdef DISCORD_RICH_PRESENCE
		activity.SetDetails(details.c_str());
		auto &assets = activity.GetAssets();
		assets.SetLargeImage("gangblanc");
		assets.SetLargeText("It's our boy");
		updateActivity(std::move(callback));
#else
		(void) callback;
#endif
		return true;
	}

	bool Discord::tick() {
		if (!core) {
			return false;
		}
#ifdef DISCORD_RICH_PRESENCE
		core->RunCallbacks();
#endif
		return true;
	}

	bool Discord::setActivityDetails(const char *new_details, bool update) {
		details = new_details;
#ifdef DISCORD_RICH_PRESENCE
		activity.SetDetails(new_details);
#endif
		return update? updateActivity() : true;
	}

	bool Discord::setActivityDetails(std::string new_details, bool update) {
		details = std::move(new_details);
#ifdef DISCORD_RICH_PRESENCE
		activity.SetDetails(details.c_str());
#endif
		return update? updateActivity() : true;
	}

	bool Discord::setActivityStartTime(std::chrono::system_clock::time_point when, bool update) {
		startingTime = when;
#ifdef DISCORD_RICH_PRESENCE
		activity.GetTimestamps().SetStart(std::chrono::duration_cast<std::chrono::seconds>(when.time_since_epoch()).count());
#endif
		return update? updateActivity() : true;
	}

	bool Discord::setActivityStartTime(bool update) {
		return setActivityStartTime(std::chrono::system_clock::now(), update);
	}

	const std::string & Discord::getDetails() const {
		return details;
	}

	void Discord::reset() {
		if (!core) {
			return;
		}
		details = defaultDetails();
		core.reset();
	}

	std::string Discord::defaultDetails() {
		return "Idling";
	}

	Discord richPresence;
}
