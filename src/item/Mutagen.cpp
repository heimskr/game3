#include "entity/Sheep.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/Mutagen.h"
#include "realm/Realm.h"
#include "types/Position.h"

namespace Game3 {
	bool Mutagen::use(Slot, const ItemStackPtr &, const Place &place, Modifiers modifiers, std::pair<float, float>) {
		RealmPtr realm = place.realm;
		assert(realm->getSide() == Side::Server);

		if (modifiers.empty() || modifiers.onlySuper())
			return false;

		for (const EntityPtr &entity: realm->findEntities(place.position)) {
			if (auto sheep = std::dynamic_pointer_cast<Sheep>(entity)) {
				if (modifiers.ctrl)
					sheep->hue.mutate(.5f);

				if (modifiers.shift)
					sheep->saturation.mutate(.5f);

				if (modifiers.alt)
					sheep->valueMultiplier.mutate(.5f);

				sheep->sendToVisible();
			}
		}

		return true;
	}
}
