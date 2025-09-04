#pragma once

#include "threading/Atomic.h"
#include "threading/Lockable.h"
#include "types/ChunkPosition.h"

#include <atomic>
#include <map>

namespace Game3 {
	class Game;
	class Village;
	struct Place;
	struct VillageOptions;

	using VillagePtr = std::shared_ptr<Village>;

	class OwnsVillages {
		public:
			OwnsVillages() = default;
			virtual ~OwnsVillages() = default;

			VillageID getNewVillageID();
			VillagePtr getVillage(VillageID id) const;
			VillagePtr addVillage(Game &, ChunkPosition, const Place &, const VillageOptions &);
			VillagePtr addVillage(Game &, VillageID, std::string name, RealmID, ChunkPosition, const Position &, Resources = {});
			inline const auto & getVillageMap() { return villageMap; }

		protected:
			virtual void associateWithRealm(const VillagePtr &, RealmID) = 0;

		private:
			Lockable<std::map<VillageID, VillagePtr>> villageMap;
			Atomic<VillageID> lastVillageID = 0;

		friend class GameDB;
	};
}
