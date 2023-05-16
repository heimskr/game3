#pragma once

#include <mutex>

#include "game/Game.h"
#include "util/MTQueue.h"

namespace Game3 {
	class LocalServer;
	class Packet;
	class RemoteClient;

	class ServerGame: public Game {
		public:
			std::unordered_set<PlayerPtr> players;
			std::shared_ptr<LocalServer> server;

			ServerGame(std::shared_ptr<LocalServer> server_):
				server(std::move(server_)) {}

			void tick();
			void broadcastTileUpdate(RealmID, Layer, const Position &, TileID);

			Side getSide() const override { return Side::Server; }
			void queuePacket(std::shared_ptr<RemoteClient>, std::shared_ptr<Packet>);
			void runCommand(RemoteClient &, const std::string &, GlobalID);
			void entityTeleported(const EntityPtr &);

		private:
			std::shared_mutex playersMutex;
			MTQueue<std::pair<std::shared_ptr<RemoteClient>, std::shared_ptr<Packet>>> packetQueue;

			inline auto lockPlayersShared() { return std::shared_lock(playersMutex); }
			inline auto lockPlayersUnique() { return std::unique_lock(playersMutex); }

			void handlePacket(RemoteClient &, Packet &);
			std::tuple<bool, std::string> commandHelper(RemoteClient &, const std::string &);
	};
}
