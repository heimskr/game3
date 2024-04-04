#pragma once

#include "data/GameDB.h"
#include "entity/ServerPlayer.h"
#include "game/Fluids.h"
#include "game/Game.h"
#include "net/RemoteClient.h"
#include "threading/Lockable.h"
#include "threading/MTQueue.h"
#include "threading/ThreadPool.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace Game3 {
	class Packet;
	class RemoteClient;
	class Server;

	class ServerGame: public Game {
		public:
			constexpr static float GARBAGE_COLLECTION_TIME = 60;

			Lockable<std::unordered_set<ServerPlayerPtr>> players;
			Lockable<std::unordered_map<std::string, ServerPlayerPtr>> playerMap;
			Lockable<std::map<std::string, ssize_t>> gameRules;
			std::weak_ptr<Server> weakServer;
			float lastGarbageCollection = 0;

			ServerGame(const std::shared_ptr<Server> &, size_t pool_size);
			~ServerGame() override;

			void init();
			void stop();
			double getFrequency() const override;
			void addEntityFactories() override;
			bool tick() final;
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
			void addPlayer(const ServerPlayerPtr &);
			bool hasPlayer(const std::string &username) const;
			void queueRemoval(const ServerPlayerPtr &);
			void openDatabase(std::filesystem::path);
			void broadcast(const Packet &, bool include_non_players = false);
			void releasePlayer(const std::string &username, const Place &);
			void setRule(const std::string &, ssize_t);
			std::optional<ssize_t> getRule(const std::string &) const;
			void removeRealm(RealmPtr) override;
			bool compareToken(Token);
			Token getOmnitoken() const;

			std::shared_ptr<ServerGame> getSelf() { return std::static_pointer_cast<ServerGame>(shared_from_this()); }
			std::shared_ptr<const ServerGame> getSelf() const { return std::static_pointer_cast<const ServerGame>(shared_from_this()); }

			inline auto getServer() const {
				auto out = weakServer.lock();
				assert(out);
				return out;
			}

			inline GameDB & getDatabase() {
				assert(database);
				return *database;
			}

			template <typename P>
			void broadcast(const Place &place, const P &packet) {
				auto lock = players.sharedLock();
				for (const ServerPlayerPtr &player: players)
					if (player->canSee(place.realm->id, place.position))
						if (std::shared_ptr<RemoteClient> client = player->toServer()->weakClient.lock())
							client->send(packet);
			}

			static Token generateRandomToken();

		private:
			MTQueue<std::pair<std::weak_ptr<RemoteClient>, std::shared_ptr<Packet>>> packetQueue;
			MTQueue<std::weak_ptr<ServerPlayer>> playerRemovalQueue;
			double timeSinceTimeUpdate = 0;
			ThreadPool pool;
			std::unique_ptr<GameDB> database;
			Token omnitoken = generateRandomToken();

			void handlePacket(RemoteClient &, Packet &);
			std::tuple<bool, std::string> commandHelper(RemoteClient &, const std::string &);
	};

	using ServerGamePtr = std::shared_ptr<ServerGame>;
}
