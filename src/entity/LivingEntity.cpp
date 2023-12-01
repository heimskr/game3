#include "entity/LivingEntity.h"

namespace Game3 {
	LivingEntity::LivingEntity():
		Entity("base:invalid/LivingEntity") {}

	void LivingEntity::onCreate() {
		health = getMaxHealth();
	}

	void LivingEntity::toJSON(nlohmann::json &json) const {
		auto this_lock = sharedLock();
		json["health"] = health;
	}

	void LivingEntity::absorbJSON(Game &, const nlohmann::json &json) {
		if (json.is_null())
			return;

		auto this_lock = uniqueLock();

		if (auto iter = json.find("health"); iter != json.end())
			health = *iter;
	}

	void LivingEntity::encode(Buffer &buffer) {
		auto this_lock = sharedLock();
		buffer << health;
	}

	void LivingEntity::decode(Buffer &buffer) {
		auto this_lock = uniqueLock();
		buffer >> health;
	}
}
