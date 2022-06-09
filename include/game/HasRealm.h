#pragma once

#include <memory>

#include "Position.h"

namespace Game3 {
	class Realm;

	struct HasRealm {
		virtual ~HasRealm() = default;
		virtual std::shared_ptr<Realm> getRealm() const = 0;
		virtual const Position & getPosition() const = 0;
	};
}
