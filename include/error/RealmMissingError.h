#pragma once

#include <format>
#include <stdexcept>

#include "types/Types.h"

namespace Game3 {
	struct RealmMissingError: std::runtime_error {
		RealmID realmID;

		RealmMissingError(RealmID realmID):
			std::runtime_error(std::format("Realm {} is missing", realmID)),
			realmID(realmID) {}
	};
}
