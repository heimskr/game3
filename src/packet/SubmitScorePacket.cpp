#include "game/ClientGame.h"
#include "packet/SubmitScorePacket.h"

namespace Game3 {
	SubmitScorePacket::SubmitScorePacket(Identifier minigame_id, uint64_t score):
		minigameID(std::move(minigame_id)), score(score) {}

	void SubmitScorePacket::handle(const std::shared_ptr<ServerGame> &, GenericClient &) {
		assert(!"TODO: implement");
	}
}
