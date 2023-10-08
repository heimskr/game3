#include "Position.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "graphics/Tileset.h"
#include "item/Pickaxe.h"
#include "realm/Realm.h"
#include "tile/Tile.h"

namespace Game3 {
	bool Pickaxe::use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>) {
		return false;
	}
}
