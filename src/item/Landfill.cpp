#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "item/Landfill.h"
#include "realm/Realm.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"

namespace Game3 {
	Landfill::Landfill(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count, Identifier terrain_name, Identifier submerged_name, ItemCount required_count):
	Item(std::move(id_), std::move(name_), base_price, max_count),
	terrainName(std::move(terrain_name)),
	submergedName(std::move(submerged_name)),
	requiredCount(required_count) {
		if (!submergedName)
			submergedName = terrainName;
	}

	bool Landfill::use(Slot slot, ItemStack &stack, const Place &place, Modifiers modifiers, std::pair<float, float>) {
		PlayerPtr player = place.player;
		RealmPtr  realm  = place.realm;

		Layer layer = modifiers.onlyShift()? Layer::Submerged : Layer::Terrain;

		if (std::optional<TileID> tile_id = place.get(layer); tile_id && *tile_id != 0)
			return false;

		if (stack.count < requiredCount)
			return false;

		player->getInventory()->decrease(stack, slot, requiredCount);
		place.set(layer, layer == Layer::Submerged? submergedName : terrainName);
		return true;
	}

	bool Landfill::drag(Slot slot, ItemStack &stack, const Place &place, Modifiers modifiers) {
		return use(slot, stack, place, modifiers, {0.f, 0.f});
	}

	bool Landfill::canUseOnWorld() const {
		return false;
	}
}
