#pragma once

#include "game/HasTickQueue.h"
#include "game/OwnsVillages.h"
#include "net/Buffer.h"
#include "realm/Realm.h"
#include "registry/Registries.h"
#include "registry/Registry.h"
#include "threading/Lockable.h"
#include "types/TickArgs.h"
#include "types/Types.h"

#include <chrono>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <shared_mutex>
#include <unordered_map>
#include <utility>
#include <variant>

#include <boost/json/fwd.hpp>

namespace Game3 {
	class Window;
	class ClientGame;
	class Menu;
	class Plantable;
	class Player;
	class Server;
	class ServerGame;
	class Tile;
	class Tileset;
	struct InteractionSet;

	class Game: public std::enable_shared_from_this<Game>, public OwnsVillages, public BufferContext, public HasTickQueue<const TickArgs &> {
		public:
			/** Seconds since the last tick */
			float delta = 0.f;
			std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
			bool debugMode = false;
			/** 12 because the game starts at noon */
			float hourOffset = 12.;
			std::atomic<double> time = 0.f;
			size_t cavesGenerated = 0;
			size_t randomTicksPerChunk = 2;
			bool dying = false;
			std::atomic_bool tickingPaused = false;

			std::map<RealmType, std::shared_ptr<InteractionSet>> interactionSets;
			std::map<Identifier, std::unordered_set<std::shared_ptr<Item>>> itemsByAttribute;

			RegistryRegistry registries;

			Lockable<std::unordered_map<GlobalID, std::weak_ptr<Agent>>> allAgents;

			virtual ~Game();

			template <typename T>
			T & registry() {
				return registries.get<T>();
			}

			template <typename T>
			const T & registry() const {
				return registries.get<const T>();
			}

			/** Returns true if ticking should continue, false if the game needs to stop. */
			virtual bool tick();
			void initRegistries();
			void addItems();
			virtual void addEntityFactories();
			void addTileEntityFactories();
			void addRealms();
			void addPacketFactories();
			void addLocalCommandFactories();
			void addTiles();
			void addFluids();
			size_t addSounds(const std::filesystem::path &);
			void addModuleFactories();
			void addMinigameFactories();
			void addStatusEffectFactories();
			virtual void initialSetup(const std::filesystem::path &dir = "gamedata");
			void initEntities();
			void initInteractionSets();
			void add(std::shared_ptr<Item>);
			void add(EntityFactory &&);
			void add(TileEntityFactory &&);
			void add(RealmFactory &&);
			void add(LocalCommandFactory &&);
			void add(ModuleFactory &&);
			void add(MinigameFactory &&);
			void add(StatusEffectFactory &&);
			void traverseData(const std::filesystem::path &);
			void loadData(const boost::json::value &);
			void addRecipe(const boost::json::value &);
			RealmID newRealmID() const;
			double getTotalSeconds() const;
			double getHour() const;
			double getMinute() const;
			/** The value to divide the color values of the tilemap pixels by. Based on the time of day. */
			double getDivisor() const;
			std::optional<TileID> getFluidTileID(FluidID);
			std::shared_ptr<Fluid> getFluid(FluidID) const;
			std::shared_ptr<Fluid> getFluid(const Identifier &) const;
			std::shared_ptr<Tile> getTile(const Identifier &);
			const std::filesystem::path * getSound(const Identifier &);
			RealmPtr tryRealm(RealmID) const;
			RealmPtr getRealm(RealmID) const;
			/** Tries to return the realm corresponding to a given realm ID. If one doesn't exist, one is created via the given function, added to the realm container and returned. */
			RealmPtr getRealm(RealmID, const std::function<RealmPtr()> &);
			void addRealm(RealmID, RealmPtr);
			void addRealm(RealmPtr);
			bool hasRealm(RealmID) const;
			virtual void removeRealm(RealmPtr);
			ItemPtr getItem(const ItemID &) const;
			bool canLog(int level) const;

			virtual Side getSide() const = 0;

			inline void clearFluidCache() { fluidCache.clear(); }

			using GameArgument = std::variant<std::shared_ptr<Window>, std::pair<std::shared_ptr<Server>, size_t>>;

			static std::shared_ptr<Game> create(Side, const GameArgument &);
			static std::shared_ptr<Game> fromJSON(Side, const boost::json::value &, const GameArgument &);

			ClientGame & toClient();
			const ClientGame & toClient() const;
			std::shared_ptr<ClientGame> toClientPointer();

			ServerGame & toServer();
			const ServerGame & toServer() const;
			std::shared_ptr<ServerGame> toServerPointer();

			template <typename T = Agent>
			std::shared_ptr<T> getAgent(GlobalID gid) {
				auto shared_lock = allAgents.sharedLock();
				if (auto iter = allAgents.find(gid); iter != allAgents.end()) {
					if (auto agent = iter->second.lock())
						return std::dynamic_pointer_cast<T>(agent);
					// This should *probably* not result in a bad data race in practice...
					shared_lock.unlock();
					auto unique_lock = allAgents.uniqueLock();
					allAgents.erase(gid);
				}

				return nullptr;
			}

		protected:
			std::chrono::system_clock::time_point lastTime = startTime;
			Lockable<std::unordered_map<RealmID, RealmPtr>> realms;

			Game();

			void associateWithRealm(const VillagePtr &, RealmID) override;

		private:
			std::unordered_map<FluidID, TileID> fluidCache;

		public:
			std::shared_ptr<FluidRegistry> fluidRegistry;
			std::shared_ptr<ItemRegistry> itemRegistry;
			std::shared_ptr<TileRegistry> tileRegistry;

			template <typename Fn>
			void iterateRealms(const Fn &function) const {
				auto lock = realms.sharedLock();
				for (const auto &[realm_id, realm]: realms)
					function(realm);
			}

		friend class GameDB;
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const Game &);

	using GamePtr = std::shared_ptr<Game>;
}
