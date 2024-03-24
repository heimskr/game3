#pragma once

#include "container/RectangularVector.h"
#include "realm/Realm.h"
#include "threading/Lockable.h"

#include <atomic>

namespace Game3 {
	class Inventory;

	class Cave: public Realm {
		public:
			static Identifier ID() { return {"base", "realm/cave"}; }
			RealmID parentRealm;
			std::atomic_size_t entranceCount = 1;

			void clearLighting(float) override;
			void onRemove() override;
			void reveal(const Position &, bool force = false);
			void generateChunk(const ChunkPosition &) override;
			bool canSpawnMonsters() const override { return true; }
			RectangularVector<uint16_t> & getOreVoronoi(ChunkPosition, std::unique_lock<DefaultMutex> &, std::default_random_engine &);

		friend class Realm;

		protected:
			Lockable<std::map<ChunkPosition, Lockable<RectangularVector<uint16_t>>>> oreVoronoi;

			using Realm::Realm;

			Cave() = delete;
			Cave(const std::shared_ptr<Game> &, RealmID, RealmID parent_realm, int seed_);

			void absorbJSON(const nlohmann::json &, bool full_data) override;
			void toJSON(nlohmann::json &, bool full_data) const override;
	};
}
