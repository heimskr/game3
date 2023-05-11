#include "game/ServerGame.h"
#include "net/GameClient.h"
#include "net/GameServer.h"
#include "packet/TileUpdatePacket.h"
#include "util/Util.h"

namespace Game3 {
	void ServerGame::tick() {
		auto now = getTime();
		auto difference = now - lastTime;
		lastTime = now;
		delta = std::chrono::duration_cast<std::chrono::nanoseconds>(difference).count() / 1'000'000'000.;
		for (auto &[id, realm]: realms)
			realm->tick(delta);
		for (const auto &player: players)
			player->ticked = false;
	}

	void ServerGame::broadcastTileUpdate(RealmID realm_id, Layer layer, const Position &position, TileID tile_id) {
		auto lock = lockPlayersShared();
		for (const auto &player: players) {
			if (player->canSee(realm_id, position)) {
				TileUpdatePacket packet(realm_id, layer, position, tile_id);
				player->client->send(packet);
			}
		}
	}
}
