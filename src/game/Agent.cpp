#include "game/Agent.h"
#include "game/ClientGame.h"
#include "game/Game.h"
#include "game/ServerGame.h"
#include "net/LocalClient.h"
#include "packet/UpdateAgentFieldPacket.h"
#include "realm/Realm.h"
#include "util/ConstexprHash.h"
#include "util/Log.h"

#include <mutex>
#include <random>

namespace Game3 {
	static std::mt19937_64 agent_rng;
	static std::mutex agent_rng_mutex;
	static bool agent_rng_seeded = false;

	std::vector<ChunkPosition> Agent::getVisibleChunks() const {
		ChunkPosition original_position = getPosition().getChunk();
		std::vector<ChunkPosition> out;
		out.reserve(REALM_DIAMETER * REALM_DIAMETER);
		constexpr int32_t half = REALM_DIAMETER / 2;
		for (int32_t y = -half; y <= half; ++y) {
			for (int32_t x = -half; x <= half; ++x) {
				out.emplace_back(ChunkPosition{x + original_position.x, y + original_position.y});
			}
		}
		return out;
	}

	bool Agent::onInteractOn(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) {
		return false;
	}

	bool Agent::onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) {
		return false;
	}

	void Agent::handleMessage(const std::shared_ptr<Agent> &, const std::string &name, std::any &) {
		throw std::runtime_error("Agent of type " + DEMANGLE(*this) + " has no message handler and can't handle message with name \"" + name + '"');
	}

	void Agent::sendMessage(const std::shared_ptr<Agent> &destination, const std::string &name, std::any &data) {
		assert(getSide() != Side::Client);
		destination->handleMessage(shared_from_this(), name, data);
	}

	bool Agent::setField(uint32_t field_name, Buffer &field_value, const PlayerPtr &) {
		if (getSide() == Side::Server) {
			// Don't allow clients to change globalID
			return false;
		}

		switch (field_name) {
			AGENT_FIELD(globalID, true);
			default:
				return false;
		}
	}

	bool Agent::validateGID(GlobalID gid) {
		return gid != 0 && gid != static_cast<GlobalID>(-1);
	}

	GlobalID Agent::generateGID() {
		std::unique_lock lock(agent_rng_mutex);

		if (!agent_rng_seeded) {
			std::random_device hardware_rng;
			uint_fast64_t seed = hardware_rng();
			seed |= uint_fast64_t(hardware_rng()) << 32;
			agent_rng.seed(seed);
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
