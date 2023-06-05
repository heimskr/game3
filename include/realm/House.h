#pragma once

#include "realm/Realm.h"

namespace Game3 {
	class House: public Realm {
		public:
			static Identifier ID() { return {"base", "realm/house"}; }

			void generateChunk(const ChunkPosition &chunk_position) override { tileProvider.updateChunk(chunk_position); }

		protected:
			using Realm::Realm;
	};
}
