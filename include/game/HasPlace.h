#pragma once

#include "types/Position.h"
#include "types/ChunkPosition.h"
#include "game/HasRealm.h"

namespace Game3 {
	struct HasPlace: HasRealm {
		virtual Position getPosition() const = 0;

		virtual Place getPlace() {
			return {getPosition(), getRealm(), {}};
		}

		virtual ChunkRange getRange() const;
	};
}
