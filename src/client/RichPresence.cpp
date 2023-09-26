#include "Log.h"
#include "client/RichPresence.h"

namespace Game3 {
	Discord::~Discord() {
		reset();
	}

	bool Discord::init(discord::Result *result_out) {
#ifdef NO_DISCORD
		(void) result_out;
		return false;
#else
		discord::Core *core_raw{};
		const discord::Result result = discord::Core::Create(1155446965744709672, DiscordCreateFlags_NoRequireDiscord, &core_raw);

		if (result_out != nullptr)
			*result_out = result;

		if (result != discord::Result::Ok) {
			ERROR("Couldn't initialize Discord: " << int(result));
			return false;
		}

		core.reset(core_raw);
		return true;
#endif
	}

	std::function<void(discord::Result)> Discord::makeActivityCallback(std::function<void(discord::Result)> callback) const {
#ifndef NO_DISCORD
		return [callback = std::move(callback)](discord::Result result) {
			if (result != discord::Result::Ok)
				ERROR("Couldn't set activity: " << int(result));
			if (callback)
				callback(result);
		};
#else
		(void) callback;
		return {};
#endif
	}

	bool Discord::updateActivity(std::function<void(discord::Result)> callback) {
#ifndef NO_DISCORD
		if (!core)
			return false;

		core->ActivityManager().UpdateActivity(activity, makeActivityCallback(std::move(callback)));
		return true;
#else
		(void) callback;
		return false;
#endif
	}


	bool Discord::initActivity(std::function<void(discord::Result)> callback) {
		if (!core)
			return false;

		details = defaultDetails();
#ifndef NO_DISCORD
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
		if (!core)
			return false;
#ifndef NO_DISCORD
		core->RunCallbacks();
#endif
		return true;
	}

	bool Discord::setActivityDetails(const char *new_details, bool update) {
		details = new_details;
#ifndef NO_DISCORD
		activity.SetDetails(new_details);
#endif
		return update? updateActivity() : true;
	}

	bool Discord::setActivityDetails(std::string new_details, bool update) {
		details = std::move(new_details);
#ifndef NO_DISCORD
		activity.SetDetails(details.c_str());
#endif
		return update? updateActivity() : true;
	}

	bool Discord::setActivityStartTime(std::chrono::system_clock::time_point when, bool update) {
		startingTime = when;
#ifndef NO_DISCORD
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
		if (!core)
			return;
		details = defaultDetails();
		core.reset();
	}

	std::string Discord::defaultDetails() {
		return "Idling";
	}

	Discord richPresence;
}
