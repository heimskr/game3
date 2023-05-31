#pragma once

#include <mutex>

#include "game/Game.h"
#include "threading/MTQueue.h"

namespace Game3 {
	class LocalServer;
	class Packet;
	class RemoteClient;

	class ServerGame: public Game {
		public:
			constexpr static float GARBAGE_COLLECTION_TIME = 60.f;

			std::unordered_set<ServerPlayerPtr> players;
			std::shared_ptr<LocalServer> server;
			float lastGarbageCollection = 0.f;

			ServerGame(std::shared_ptr<LocalServer> server_):
				server(std::move(server_)) {}

			void addEntityFactories() override;
			void tick();
			void garbageCollect();
			void broadcastTileUpdate(RealmID, Layer, const Position &, TileID);
			Side getSide() const override { return Side::Server; }
			void queuePacket(std::shared_ptr<RemoteClient>, std::shared_ptr<Packet>);
			void runCommand(RemoteClient &, const std::string &, GlobalID);
			void entityTeleported(Entity &);
			void entityDestroyed(const Entity &);
			void tileEntityDestroyed(const TileEntity &);
			void remove(const ServerPlayerPtr &);
			void queueRemoval(const ServerPlayerPtr &);

			inline auto lockPlayersShared() { return std::shared_lock(playersMutex); }
			inline auto lockPlayersUnique() { return std::unique_lock(playersMutex); }

		private:
			std::shared_mutex playersMutex;
			MTQueue<std::pair<std::shared_ptr<RemoteClient>, std::shared_ptr<Packet>>> packetQueue;
			MTQueue<ServerPlayerPtr> playerRemovalQueue;

			void handlePacket(RemoteClient &, Packet &);
			std::tuple<bool, std::string> commandHelper(RemoteClient &, const std::string &);
	};
}
