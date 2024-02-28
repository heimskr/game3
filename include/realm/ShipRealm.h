#pragma once

#include "realm/Realm.h"
#include "worldgen/WorldGen.h"

namespace Game3 {
	class ShipRealm: public Realm {
		public:
			static Identifier ID() { return "base:realm/ship"; }

			WorldGenParams worldgenParams;

			using Realm::Realm;

			void generateChunk(const ChunkPosition &) override;

		protected:
			void absorbJSON(const nlohmann::json &, bool full_data) override;
			void toJSON(nlohmann::json &, bool full_data) const override;
	};
}
