#pragma once

#include "Position.h"
#include "game/ChunkPosition.h"
#include "game/HasRealm.h"

namespace Game3 {
	struct HasPlace: HasRealm {
		virtual const Position & getPosition() const = 0;

		virtual Place getPlace() {
			return {getPosition(), getRealm(), {}};
		}

		virtual ChunkRange getRange() const;
	};
}
