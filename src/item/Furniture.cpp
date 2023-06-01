#include "Position.h"
#include "Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/Furniture.h"
#include "realm/Realm.h"
#include "tileentity/Ghost.h"

namespace Game3 {
	bool Furniture::use(Slot slot, ItemStack &stack, const Place &place, Modifiers) {
		auto &realm = *place.realm;
		Game &game = realm.getGame();
		const auto &position = place.position;
		const auto &details = GhostDetails::get(game, stack);

		if (auto id = place.get(details.layer); !id || *id != realm.getTileset().getEmptyID())
			return false;

		if (!realm.hasTileEntityAt(position) && nullptr != realm.add(TileEntity::create<Ghost>(game, place, stack.withCount(1)))) {
			place.realm->confirmGhosts(); // lol
			if (--stack.count == 0)
				place.player->inventory->erase(slot);
			place.player->inventory->notifyOwner();
			return true;
		}

		return false;
	}
}
