#pragma once

#include "realm/Realm.h"
#include "worldgen/WorldGen.h"

namespace Game3 {
	class ShipRealm: public Realm {
		public:
			static Identifier ID() { return "base:realm/ship"; }

			GlobalID shipID{};
			WorldGenParams worldgenParams;

			void generateChunk(const ChunkPosition &) override;

		protected:
			using Realm::Realm;

			ShipRealm() = delete;
			ShipRealm(const std::shared_ptr<Game> &, RealmID, GlobalID ship_id, int seed_);

			void absorbJSON(const nlohmann::json &, bool full_data) override;
			void toJSON(nlohmann::json &, bool full_data) const override;

		friend class Realm;
	};
}
