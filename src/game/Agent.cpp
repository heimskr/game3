#include <mutex>
#include <random>

#include "Log.h"
#include "game/Agent.h"
#include "game/ClientGame.h"
#include "game/Game.h"
#include "net/LocalClient.h"
#include "realm/Realm.h"

namespace Game3 {
	static std::mt19937_64 agent_rng;
	static std::mutex agent_rng_mutex;
	static bool agent_rng_seeded = false;

	std::vector<ChunkPosition> Agent::getVisibleChunks() const {
		ChunkPosition original_position = getChunkPosition(getPosition());
		std::vector<ChunkPosition> out;
		out.reserve(REALM_DIAMETER * REALM_DIAMETER);
		constexpr int32_t half = REALM_DIAMETER / 2;
		for (int32_t y = -half; y <= half; ++y)
			for (int32_t x = -half; x <= half; ++x)
				out.emplace_back(ChunkPosition{x + original_position.x, y + original_position.y});
		return out;
	}

	void Agent::handleMessage(const std::shared_ptr<Agent> &, const std::string &, std::any &) {
		throw std::runtime_error("Agent of type " + std::string(typeid(*this).name()) + " has no message handler");
	}

	void Agent::sendMessage(const std::shared_ptr<Agent> &destination, const std::string &name, std::any &data) {
		assert(getSide() != Side::Client);
		destination->handleMessage(shared_from_this(), name, data);
	}

	bool Agent::validateGID(GlobalID gid) {
		return gid != 0 && gid != static_cast<GlobalID>(-1);
	}

	GlobalID Agent::generateGID() {
		std::unique_lock lock(agent_rng_mutex);

		if (!agent_rng_seeded) {
			std::random_device hardware_rng;
			agent_rng.seed(hardware_rng());
			agent_rng_seeded = true;
		}

		return static_cast<GlobalID>(agent_rng());
	}

	bool Agent::hasBeenSentTo(const std::shared_ptr<Player> &player) {
		auto lock = sentTo.sharedLock();
		return sentTo.contains(player);
	}

	void Agent::onSend(const std::shared_ptr<Player> &player) {
		auto lock = sentTo.uniqueLock();
		sentTo.emplace(player);
	}
}
