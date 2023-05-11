#pragma once

#include <mutex>

#include "game/Game.h"

namespace Game3 {
	class LocalServer;

	class ServerGame: public Game {
		public:
			std::unordered_set<PlayerPtr> players;
			std::shared_ptr<LocalServer> server;

			ServerGame(std::shared_ptr<LocalServer> server_):
				server(std::move(server_)) {}

			void tick();
			void broadcastTileUpdate(RealmID, Layer, const Position &, TileID);

			Side getSide() const override { return Side::Server; }
			void runCommand(const PlayerPtr &, const std::string &, GlobalID);

		private:
			std::shared_mutex playersMutex;

			inline auto lockPlayersShared() { return std::shared_lock(playersMutex); }
			inline auto lockPlayersUnique() { return std::unique_lock(playersMutex); }

			std::tuple<bool, std::string> commandHelper(const PlayerPtr &, const std::string &);
	};
}
