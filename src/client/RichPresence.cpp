#include "Log.h"
#include "client/RichPresence.h"

namespace Game3 {
	Discord::~Discord() {
		reset();
	}

	bool Discord::init(discord::Result *result_out) {
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
	}

	std::function<void(discord::Result)> Discord::makeActivityCallback(std::function<void(discord::Result)> callback) const {
		return [callback = std::move(callback)](discord::Result result) {
			if (result != discord::Result::Ok)
				ERROR("Couldn't set activity: " << int(result));
			if (callback)
				callback(result);
		};
	}

	bool Discord::updateActivity(std::function<void(discord::Result)> callback) {
		if (!core)
			return false;

		core->ActivityManager().UpdateActivity(activity, makeActivityCallback(std::move(callback)));
		return true;
	}


	bool Discord::initActivity(std::function<void(discord::Result)> callback) {
		if (!core)
			return false;

		details = defaultDetails();
		activity.SetDetails(details.c_str());
		auto &assets = activity.GetAssets();
		assets.SetLargeImage("gangblanc");
		assets.SetLargeText("It's our boy");
		updateActivity(std::move(callback));
		return true;
	}

	bool Discord::tick() {
		if (!core)
			return false;
		core->RunCallbacks();
		return true;
	}

	bool Discord::setActivityDetails(const char *new_details) {
		details = new_details;
		activity.SetDetails(new_details);
		return updateActivity();
	}

	bool Discord::setActivityDetails(std::string new_details) {
		details = std::move(new_details);
		activity.SetDetails(details.c_str());
		return updateActivity();
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
