#include "Extractors.h"
#include "Log.h"
#include "Position.h"
#include "Quadrant.h"
#include "Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/ItemPipeItem.h"
#include "realm/Realm.h"
#include "tileentity/ItemPipe.h"

namespace Game3 {
	ItemPipeItem::ItemPipeItem(MoneyCount base_price):
		Item("base:item/item_pipe"_id, "Item Pipe", base_price, 64) {}

	bool ItemPipeItem::use(Slot slot, ItemStack &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets) {
		auto &realm = *place.realm;

		auto tile_entity = realm.tileEntityAt(place.position);
		if (!tile_entity) {
			if (modifiers.onlyShift())
				return false;

			if (--stack.count == 0)
				place.player->inventory->erase(slot);
			else
				place.player->inventory->notifyOwner();

			place.realm->add(TileEntity::create<ItemPipe>(realm.getGame(), place.position));
			return true;
		}

		auto pipe = std::dynamic_pointer_cast<Pipe>(tile_entity);
		if (!pipe)
			return false;

		if (modifiers.onlyShift()) {
			place.player->give(ItemStack(place.getGame(), shared_from_this(), 1));
			realm.queueDestruction(pipe);
			return true;
		}

		const auto [x, y] = offsets;
		const Quadrant quadrant = getQuadrant(x, y);

		// Hold ctrl to toggle extractors.
		if (modifiers.onlyCtrl()) {
			if (pipe->getDirections()[quadrant])
				pipe->getExtractors().toggle(quadrant);
		} else if (!pipe->getDirections().toggle(quadrant)) {
			pipe->getExtractors()[quadrant] = false;
		}

		pipe->increaseUpdateCounter();
		pipe->broadcast();
		return true;
	}
}
