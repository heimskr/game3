#pragma once

#include "data/Richness.h"
#include "game/HasGame.h"
#include "game/Tickable.h"
#include "types/ChunkPosition.h"
#include "types/TickArgs.h"
#include "types/VillageOptions.h"

#include <memory>
#include <optional>
#include <string>

namespace Game3 {
	class ServerGame;

	class Village: public Tickable, public HasGame {
		public:
			Village() = default;
			Village(ServerGame &, const Place &, const VillageOptions &);
			Village(ServerGame &, RealmID, ChunkPosition, const Position &, const VillageOptions &);
			Village(ServerGame &, VillageID, RealmID, ChunkPosition, const Position &, const VillageOptions &);
			Village(VillageID, RealmID, std::string name_, ChunkPosition, const Position &, const VillageOptions &, Richness, Resources);

			inline auto getID() const { return id; }
			inline auto getRealmID() const { return realmID; }
			inline auto getChunkPosition() const { return chunkPosition; }
			inline auto getPosition() const { return position; }
			inline const auto & getName() const { return name; }
			inline const auto & getOptions() const { return options; }
			inline const auto & getRichness() const { return richness; }
			inline const auto & getResources() const { return resources; }

			std::optional<double> getRichness(const Identifier &);

			Tick enqueueTick() override;
			void tick(const TickArgs &);

			Game & getGame() override;

			static std::string getSQL();

		private:
			VillageID id{};
			std::string name;
			RealmID realmID{};
			ChunkPosition chunkPosition;
			Position position;
			VillageOptions options;
			Richness richness;
			Resources resources;

			void addResources();

		friend class OwnsVillages;
	};

	using VillagePtr = std::shared_ptr<Village>;
}
