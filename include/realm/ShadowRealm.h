#pragma once

#include "realm/Realm.h"
#include "worldgen/WorldGen.h"

namespace Game3 {
	class ShadowRealm: public Realm {
		public:
			static Identifier ID() { return "base:realm/shadow"; }

			WorldGenParams worldgenParams;

			void generateChunk(const ChunkPosition &) override;

		protected:
			using Realm::Realm;

			void absorbJSON(const boost::json::value &, bool full_data) override;
			void toJSON(boost::json::value &, bool full_data) const override;
	};
}
