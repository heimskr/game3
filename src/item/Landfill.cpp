#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "item/Landfill.h"
#include "realm/Realm.h"
#include "ui/Window.h"

namespace Game3 {
	Landfill::Landfill(ItemID id, std::string name, MoneyCount basePrice, ItemCount maxCount, Layer terrainLayer, Identifier terrainName, Identifier objectsName, ItemCount requiredCount):
	Item(std::move(id), std::move(name), basePrice, maxCount),
	terrainName(std::move(terrainName)),
	objectsName(std::move(objectsName)),
	requiredCount(requiredCount),
	terrainLayer(terrainLayer) {
		if (!objectsName) {
			objectsName = terrainName;
		}
	}

	bool Landfill::use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float>) {
		PlayerPtr player = place.player;
		RealmPtr  realm  = place.realm;

		Layer layer = modifiers.onlyShift()? Layer::Objects : terrainLayer;

		if (std::optional<TileID> tile_id = place.get(layer); tile_id && *tile_id != 0) {
			return false;
		}

		if (stack->count < requiredCount) {
			return false;
		}

		player->getInventory(0)->decrease(stack, slot, requiredCount, true);
		place.set(layer, layer == Layer::Objects? objectsName : terrainName);
		return true;
	}

	bool Landfill::drag(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets, DragAction) {
		return use(slot, stack, place, modifiers, offsets);
	}

	bool Landfill::canUseOnWorld() const {
		return false;
	}
}
