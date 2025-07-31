#include "dialogue/Dialogue.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "packet/PurchaseResultPacket.h"

namespace Game3 {
	void PurchaseResultPacket::handle(const std::shared_ptr<ClientGame> &game) {
		ClientPlayerPtr player = game->getPlayer();

		player->dialogueGraph.withUnique([this](const DialogueGraphPtr &graph) {
			if (!graph) {
				return;
			}
			auto lock = graph->uniqueLock();
			graph->selectNode(success? "purchase_successful" : "purchase_failed");
		});
	}
}
