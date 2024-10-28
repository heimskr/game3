#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/Cake.h"
#include "realm/Realm.h"

namespace Game3 {
	bool Cake::use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float>) {
		RealmPtr realm = place.realm;
		PlayerPtr player = place.player;
		GamePtr game = realm->getGame();

		if (place.position == player->getPosition()) {
			return use(slot, stack, player, modifiers);
		}

		constexpr Layer layer = Layer::Objects;

		if (auto tile_id = place.get(layer); tile_id && tile_id != 0) {
			return false;
		}

		place.set(layer, "base:tile/cake");
		player->getInventory(0)->decrease(stack, slot, 1, true);
		return true;
	}
}
