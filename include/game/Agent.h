#pragma once

#include "ChunkPosition.h"
#include "game/HasPlace.h"

namespace Game3 {
	struct Agent: HasPlace {
		std::vector<ChunkPosition> getVisibleChunks() const;
		virtual GlobalID getGID() const = 0;
		virtual void setGID(GlobalID) = 0;
	};
}
