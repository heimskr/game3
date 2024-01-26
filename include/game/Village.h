#pragma once

#include "data/Richness.h"
#include "game/HasGame.h"
#include "game/Tickable.h"
#include "threading/Atomic.h"
#include "threading/Lockable.h"
#include "types/ChunkPosition.h"
#include "types/TickArgs.h"
#include "types/VillageOptions.h"

#include <memory>
#include <optional>
#include <string>

namespace Game3 {
	class ConsumptionRule;
	class Game;
	class ProductionRule;
	struct ConsumptionRuleRegistry;
	struct ProductionRuleRegistry;

	class Village: public Tickable, public HasGame {
		public:
			Village() = default;
			Village(Game &, const Place &, const VillageOptions &);
			Village(Game &, RealmID, ChunkPosition, const Position &, const VillageOptions &);
			Village(Game &, VillageID, RealmID, ChunkPosition, const Position &, const VillageOptions &);
			Village(VillageID, RealmID, std::string name_, ChunkPosition, const Position &, const VillageOptions &, Richness, Resources, LaborAmount, double);

			inline auto getID() const { return id; }
			inline auto getRealmID() const { return realmID; }
			inline auto getChunkPosition() const { return chunkPosition; }
			inline auto getPosition() const { return position; }
			inline auto getLabor() const { return labor.load(); }
			inline auto getRandomValue() const { return randomValue; }
			inline const auto & getName() const { return name; }
			inline const auto & getOptions() const { return options; }
			inline const auto & getRichness() const { return richness; }
			inline const auto & getResources() const { return resources; }

			inline void setLabor(auto value) { labor = value; }
			inline void setResources(auto value) { resources = std::move(value); }

			std::optional<double> getRichness(const Identifier &);

			Tick enqueueTick() override;
			void tick(const TickArgs &);

			Game & getGame() override;

			void addSubscriber(PlayerPtr);
			void removeSubscriber(const PlayerPtr &);
			size_t getSubscriberCount() const;

			static std::string getSQL();

		private:
			VillageID id{};
			std::string name;
			RealmID realmID{};
			ChunkPosition chunkPosition;
			Position position;
			VillageOptions options;
			Richness richness;
			Lockable<Resources> resources;
			Atomic<LaborAmount> labor{};
			double randomValue;

			Lockable<std::unordered_set<PlayerPtr>> subscribedPlayers;

			void produce(BiomeType, const ProductionRule &);
			void produce(BiomeType, const ProductionRuleRegistry &);
			bool consume(const ConsumptionRule &);
			void consume(const ConsumptionRuleRegistry &);
			void sendUpdates();

			static double chooseRandomValue();
			static Resources getDefaultResources();

		friend class OwnsVillages;
	};

	using VillagePtr = std::shared_ptr<Village>;
}
