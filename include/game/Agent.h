#pragma once

#include "ChunkPosition.h"
#include "game/HasPlace.h"
#include "threading/Lockable.h"
#include "container/WeakSet.h"

namespace Game3 {
	class Buffer;
	class Game;
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
			bool initialized = false;

			virtual ~Agent() = default;

			std::vector<ChunkPosition> getVisibleChunks() const;

			virtual Side getSide() const = 0;
			virtual Type getAgentType() const = 0;
			virtual std::string getName() = 0;

			virtual void handleMessage(Agent &source, const std::string &name, Buffer &data);
			virtual void sendMessage(Agent &destination, const std::string &name, Buffer &data);

			virtual GlobalID getGID() const { return globalID; }
			virtual void setGID(GlobalID new_gid) { globalID = new_gid; }
			inline bool hasGID() const { return globalID != static_cast<GlobalID>(-1); }
			static bool validateGID(GlobalID);

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

			bool hasBeenSentTo(const std::shared_ptr<Player> &);
			void onSend(const std::shared_ptr<Player> &);

			static GlobalID generateGID();

		private:
			Lockable<AgentMeta> agentMeta;
			Lockable<WeakSet<Player>> sentTo;
	};

	using AgentPtr = std::shared_ptr<Agent>;
}
