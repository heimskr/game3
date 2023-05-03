#pragma once

#include "realm/Realm.h"

namespace Game3 {
	class House: public Realm {
		public:
			static Identifier ID() { return {"base", "realm/house"}; }

			void generateChunk(const ChunkPosition &) override {}

		protected:
			using Realm::Realm;
	};
}
