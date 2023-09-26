#include "Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/Furniture.h"
#include "realm/Realm.h"
#include "tileentity/Ghost.h"

namespace Game3 {
	bool Furniture::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>) {
		auto &realm = *place.realm;
		Game &game = realm.getGame();
		const auto &position = place.position;
		const auto &details = GhostDetails::get(game, stack);

		if (auto id = place.get(details.layer); !id || *id != realm.getTileset().getEmptyID())
			return false;

		if (!realm.hasTileEntityAt(position) && nullptr != realm.add(TileEntity::create<Ghost>(game, place, stack.withCount(1)))) {
			place.realm->confirmGhosts(); // lol
			const InventoryPtr inventory = place.player->getInventory();
			if (--stack.count == 0)
				inventory->erase(slot);
			inventory->notifyOwner();
			return true;
		}

		return false;
	}
}
