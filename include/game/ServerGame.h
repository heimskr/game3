#pragma once

#include <mutex>

#include "game/Game.h"

namespace Game3 {
	class GameServer;

	class ServerGame: public Game {
		public:
			std::unordered_set<PlayerPtr> players;
			std::shared_ptr<GameServer> server;

			ServerGame(std::shared_ptr<GameServer> server_):
				server(std::move(server_)) {}

			void tick();
			void broadcastTileUpdate(RealmID, Layer, const Position &, TileID);

			Side getSide() const override { return Side::Server; }

		private:
			std::shared_mutex playersMutex;

			inline auto lockPlayersShared() { return std::shared_lock(playersMutex); }
			inline auto lockPlayersUnique() { return std::unique_lock(playersMutex); }
	};
}
