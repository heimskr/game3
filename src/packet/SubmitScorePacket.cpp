#include "entity/ServerPlayer.h"
#include "game/ServerGame.h"
#include "net/GenericClient.h"
#include "packet/SubmitScorePacket.h"

namespace Game3 {
	namespace {
		constexpr uint64_t POINTS_PER_TICKET = 100;
	}

	SubmitScorePacket::SubmitScorePacket(Identifier minigame_id, uint64_t score):
		minigameID(std::move(minigame_id)), score(score) {}

	void SubmitScorePacket::handle(const std::shared_ptr<ServerGame> &game, GenericClient &client) {
		if (score < POINTS_PER_TICKET) {
			return;
		}

		if (const PlayerPtr player = client.getPlayer()) {
			player->give(ItemStack::create(game, "base:item/ticket", score / POINTS_PER_TICKET));
		}
	}
}
