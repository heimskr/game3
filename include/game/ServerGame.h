#pragma once

#include "data/GameDB.h"
#include "entity/ServerPlayer.h"
#include "game/Fluids.h"
#include "game/Game.h"
#include "net/RemoteClient.h"
#include "threading/Lockable.h"
#include "threading/MTQueue.h"

#include <mutex>

namespace Game3 {
	class LocalServer;
	class Packet;
	class RemoteClient;

	class ServerGame: public Game {
		public:
			constexpr static float GARBAGE_COLLECTION_TIME = 60.f;

			Lockable<std::unordered_set<ServerPlayerPtr>> players;
			std::weak_ptr<LocalServer> weakServer;
			GameDB database{*this};
			float lastGarbageCollection = 0.f;

			ServerGame(const std::shared_ptr<LocalServer> &server_):
				weakServer(server_) {}

			~ServerGame() override;

			void addEntityFactories() override;
			void tick() final;
			void garbageCollect();
			void broadcastTileUpdate(RealmID, Layer, const Position &, TileID);
			void broadcastFluidUpdate(RealmID, const Position &, FluidTile);
			Side getSide() const override { return Side::Server; }
			void queuePacket(std::shared_ptr<RemoteClient>, std::shared_ptr<Packet>);
			void runCommand(RemoteClient &, const std::string &, GlobalID);
			void entityChangingRealms(Entity &, const RealmPtr &new_realm, const Position &new_position);
			void entityTeleported(Entity &, MovementContext);
			void entityDestroyed(const Entity &);
			void tileEntitySpawned(const TileEntityPtr &);
			void tileEntityDestroyed(const TileEntity &);
			void remove(const ServerPlayerPtr &);
			void queueRemoval(const ServerPlayerPtr &);
			void openDatabase(std::filesystem::path);

			inline auto getServer() const {
				auto out = weakServer.lock();
				assert(out);
				return out;
			}

			template <typename P>
			void broadcast(const Place &place, const P &packet) {
				auto lock = players.sharedLock();
				for (const auto &player: players)
					if (player->canSee(place.realm->id, place.position))
						if (auto client = player->toServer()->weakClient.lock())
							client->send(packet);
			}

		private:
			MTQueue<std::pair<std::weak_ptr<RemoteClient>, std::shared_ptr<Packet>>> packetQueue;
			MTQueue<std::weak_ptr<ServerPlayer>> playerRemovalQueue;
			double timeSinceTimeUpdate = 0.;

			void handlePacket(RemoteClient &, Packet &);
			std::tuple<bool, std::string> commandHelper(RemoteClient &, const std::string &);
	};
}
