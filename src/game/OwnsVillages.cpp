#include "game/OwnsVillages.h"
#include "game/Game.h"
#include "lib/JSON.h"
#include "realm/Realm.h"

#include <optional>

namespace Game3 {
	VillageID OwnsVillages::getNewVillageID() {
		return ++lastVillageID;
	}

	VillagePtr OwnsVillages::getVillage(VillageID id) const {
		return villageMap.at(id);
	}

	VillagePtr OwnsVillages::addVillage(Game &game, ChunkPosition chunk_position, const Place &place, const VillageOptions &options) {
		auto lock = villageMap.uniqueLock();
		const auto new_id = getNewVillageID();
		VillagePtr new_village = std::make_shared<Village>(game, new_id, place.realm->getID(), chunk_position, place.position, options);
		villageMap[new_id] = new_village;
		associateWithRealm(new_village, place.realm->getID());
		return new_village;
	}

	VillagePtr OwnsVillages::addVillage(Game &game, VillageID village_id, std::string name, RealmID realm_id, ChunkPosition chunk_position, const Position &position, Resources resources) {
		VillagePtr new_village = std::make_shared<Village>(village_id, realm_id, std::move(name), chunk_position, position, VillageOptions{}, Richness{}, std::move(resources), LaborAmount{}, double{}, double{});

		villageMap[village_id] = new_village;
		lastVillageID = std::max(lastVillageID.load(), village_id);

		new_village->setGame(game.shared_from_this());
		associateWithRealm(new_village, realm_id);
		return new_village;
	}
}
