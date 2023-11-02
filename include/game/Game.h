#pragma once

#include "types/Types.h"
#include "entity/Player.h"
#include "net/Buffer.h"
#include "realm/Realm.h"
#include "registry/Registries.h"
#include "registry/Registry.h"
#include "threading/Lockable.h"

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

#include <gtkmm.h>
#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class Canvas;
	class ClientGame;
	class MainWindow;
	class Menu;
	class Player;
	class Server;
	class ServerGame;
	class Tile;
	class Tileset;
	struct InteractionSet;
	struct Plantable;

	class Game: public std::enable_shared_from_this<Game>, public BufferContext {
		public:
			static constexpr const char *DEFAULT_PATH = "game.g3";

			/** Seconds since the last tick */
			float delta = 0.f;
			std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
			bool debugMode = true;
			/** 12 because the game starts at noon */
			float hourOffset = 12.;
			std::atomic<double> time = 0.f;
			Tick currentTick = 0;
			size_t cavesGenerated = 0;
			size_t randomTicksPerChunk = 2;
			bool dying = false;
			std::atomic_bool tickingPaused{false};

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
			void addModuleFactories();
			void initialSetup(const std::filesystem::path &dir = "gamedata");
			void initEntities();
			void initInteractionSets();
			void add(std::shared_ptr<Item>);
			void add(EntityFactory &&);
			void add(TileEntityFactory &&);
			void add(RealmFactory &&);
			void add(PacketFactory &&);
			void add(LocalCommandFactory &&);
			void add(ModuleFactory &&);
			void traverseData(const std::filesystem::path &);
			void loadData(const nlohmann::json &);
			void addRecipe(const nlohmann::json &);
			RealmID newRealmID() const;
			double getTotalSeconds() const;
			double getHour() const;
			double getMinute() const;
			/** The value to divide the color values of the tilemap pixels by. Based on the time of day. */
			double getDivisor() const;
			std::optional<TileID> getFluidTileID(FluidID);
			std::shared_ptr<Fluid> getFluid(FluidID) const;
			std::shared_ptr<Tile> getTile(const Identifier &);
			RealmPtr tryRealm(RealmID) const;
			RealmPtr getRealm(RealmID) const;
			/** Tries to return the realm corresponding to a given realm ID. If one doesn't exist, one is created via the given function, added to the realm container and returned. */
			RealmPtr getRealm(RealmID, const std::function<RealmPtr()> &);
			void addRealm(RealmID, RealmPtr);
			void addRealm(RealmPtr);
			bool hasRealm(RealmID) const;
			void removeRealm(RealmID);

			virtual Side getSide() const = 0;

			inline void clearFluidCache() { fluidCache.clear(); }

			using GameArgument = std::variant<Canvas *, std::pair<std::shared_ptr<Server>, size_t>>;

			static std::shared_ptr<Game> create(Side, const GameArgument &);
			static std::shared_ptr<Game> fromJSON(Side, const nlohmann::json &, const GameArgument &);

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
					if (auto agent = iter->second.lock()) {
						return std::dynamic_pointer_cast<T>(agent);
					} else {
						// This should *probably* not result in a data race in practice...
						shared_lock.unlock();
						auto unique_lock = allAgents.uniqueLock();
						allAgents.erase(gid);
					}
				}

				return nullptr;
			}

		protected:
			std::chrono::system_clock::time_point lastTime = startTime;
			Lockable<std::unordered_map<RealmID, RealmPtr>> realms;

			Game() = default;

		private:
			std::unordered_map<FluidID, TileID> fluidCache;

		public:
			template <typename Fn>
			void iterateRealms(const Fn &function) const {
				auto lock = realms.sharedLock();
				for (const auto &[realm_id, realm]: realms)
					function(realm);
			}
	};

	void to_json(nlohmann::json &, const Game &);

	using GamePtr = std::shared_ptr<Game>;
}
