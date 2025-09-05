#pragma once

#include "data/GameDB.h"
#include "entity/ServerPlayer.h"
#include "fluid/Fluid.h"
#include "game/Game.h"
#include "threading/Lockable.h"
#include "threading/MTQueue.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace Game3 {
	class GenericClient;
	class Packet;
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
			void queuePacket(std::shared_ptr<GenericClient>, std::shared_ptr<Packet>);
			void runCommand(GenericClient &, const std::string &, GlobalID);
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
			void broadcast(const PacketPtr &, bool include_non_players = false);
			void releasePlayer(const std::string &username, const Place &);
			void setRule(const std::string &, ssize_t);
			std::optional<ssize_t> getRule(const std::string &) const;
			void removeRealm(RealmPtr) override;
			bool compareToken(Token);
			Token getOmnitoken() const;
			/** Returns whether the initial worldgen happened (nothing happens if it's not needed). */
			bool initialWorldgen(size_t overworld_seed);

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

			void broadcast(const Place &, const PacketPtr &);

			static Token generateRandomToken();

		private:
			MTQueue<std::pair<std::weak_ptr<GenericClient>, std::shared_ptr<Packet>>> packetQueue;
			MTQueue<std::weak_ptr<ServerPlayer>> playerRemovalQueue;
			double timeSinceTimeUpdate = 0;
			std::unique_ptr<GameDB> database;
			Token omnitoken = generateRandomToken();
			bool databaseValid = false;

			void handlePacket(GenericClient &, Packet &);
			std::tuple<bool, std::string> commandHelper(GenericClient &, const std::string &);
	};

	using ServerGamePtr = std::shared_ptr<ServerGame>;
}
