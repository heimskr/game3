#pragma once

#include "data/Richness.h"
#include "types/ChunkPosition.h"
#include "types/VillageOptions.h"

namespace Game3 {
	class ServerGame;

	class Village {
		public:
			Village(ServerGame &, ChunkPosition, const Position &);
			Village(ServerGame &, const Position &);

			inline auto getChunkPosition() const { return chunkPosition; }
			inline auto getPosition() const { return position; }
			std::optional<double> getRichness(const Identifier &);

			static std::string getSQL();

		private:
			size_t id;
			VillageOptions options;
			ChunkPosition chunkPosition;
			Position position;
			Richness richness;
			std::map<Identifier, double> resources;

		friend class HasVillages;
	};
}
