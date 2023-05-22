#pragma once

#include "ChunkPosition.h"
#include "game/HasPlace.h"

namespace Game3 {
	struct Agent: HasPlace {
		GlobalID globalID = generateGID();

		std::vector<ChunkPosition> getVisibleChunks() const;

		virtual GlobalID getGID() const { return globalID; }
		virtual void setGID(GlobalID new_gid) { globalID = new_gid; }
		inline bool hasGID() const { return globalID != static_cast<GlobalID>(-1); }

		static GlobalID generateGID();
	};
}
