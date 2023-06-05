#pragma once

#include "ChunkPosition.h"
#include "game/HasPlace.h"

namespace Game3 {
	struct Agent: HasPlace {
		enum class Type {Entity, TileEntity};

		GlobalID globalID = generateGID();

		virtual ~Agent() = default;

		std::vector<ChunkPosition> getVisibleChunks() const;

		virtual Side getSide() const = 0;
		virtual Type getAgentType() const = 0;

		virtual GlobalID getGID() const { return globalID; }
		virtual void setGID(GlobalID new_gid) { globalID = new_gid; }
		inline bool hasGID() const { return globalID != static_cast<GlobalID>(-1); }

		static GlobalID generateGID();
	};
}
