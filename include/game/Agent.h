#pragma once

#include "ChunkPosition.h"
#include "game/HasPlace.h"
#include "threading/Lockable.h"
#include "util/WeakSet.h"

namespace Game3 {
	class Player;

	struct AgentMeta {
		UpdateCounter updateCounter = 0;
		AgentMeta() = default;
		AgentMeta(UpdateCounter counter): updateCounter(counter) {}
	};

	class Agent: public HasPlace {
		public:
			enum class Type {Entity, TileEntity};

			GlobalID globalID = generateGID();

			virtual ~Agent() = default;

			std::vector<ChunkPosition> getVisibleChunks() const;

			virtual Side getSide() const = 0;
			virtual Type getAgentType() const = 0;

			virtual GlobalID getGID() const { return globalID; }
			virtual void setGID(GlobalID new_gid) { globalID = new_gid; }
			inline bool hasGID() const { return globalID != static_cast<GlobalID>(-1); }

			inline auto getUpdateCounter() {
				auto lock = agentMeta.sharedLock();
				return agentMeta.updateCounter;
			}

			inline auto increaseUpdateCounter() {
				auto lock = agentMeta.uniqueLock();
				return ++agentMeta.updateCounter;
			}

			inline void setUpdateCounter(UpdateCounter new_counter) {
				auto lock = agentMeta.uniqueLock();
				agentMeta.updateCounter = new_counter;
			}

			static GlobalID generateGID();

			bool hasBeenSentTo(const std::shared_ptr<Player> &);
			void onSend(const std::shared_ptr<Player> &);

		private:
			Lockable<AgentMeta> agentMeta;
			Lockable<WeakSet<Player>> sentTo;
	};
}
