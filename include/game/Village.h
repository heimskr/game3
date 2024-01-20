#pragma once

#include "data/Richness.h"
#include "types/ChunkPosition.h"
#include "types/VillageOptions.h"

namespace Game3 {
	class ServerGame;

	class Village {
		public:
			Village() = default;
			Village(ServerGame &, const Place &, const VillageOptions &);
			Village(ServerGame &, RealmID, ChunkPosition, const Position &, const VillageOptions &);
			Village(size_t, RealmID, ChunkPosition, const Position &, const VillageOptions &, Richness, Resources);

			inline auto getID() const { return id; }
			inline auto getRealmID() const { return realmID; }
			inline auto getChunkPosition() const { return chunkPosition; }
			inline auto getPosition() const { return position; }
			inline const auto & getOptions() const { return options; }
			inline const auto & getRichness() const { return richness; }
			inline const auto & getResources() const { return resources; }

			std::optional<double> getRichness(const Identifier &);

			static std::string getSQL();

		private:
			size_t id;
			RealmID realmID;
			ChunkPosition chunkPosition;
			Position position;
			VillageOptions options;
			Richness richness;
			Resources resources;

		friend class HasVillages;
	};
}
