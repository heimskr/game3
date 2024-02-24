#include "types/Position.h"
#include "game/Game.h"
#include "graphics/Tileset.h"
#include "item/VoidPickaxe.h"
#include "realm/Realm.h"
#include "util/Util.h"

namespace Game3 {
	bool VoidPickaxe::use(Slot slot, ItemStack &, const Place &place, Modifiers, std::pair<double, double>) {
		PlayerPtr player = place.player;
		RealmPtr  realm  = place.realm;
		Game &game = realm->getGame();
		Tileset &tileset = realm->getTileset();

		for (const Layer layer: reverse(allLayers)) {
			std::optional<TileID> tile = place.get(layer);
			if (!tile || *tile == 0)
				continue;

			ItemStack stack;
			if (tileset.getItemStack(game, tileset[*tile], stack))
				player->give(stack, slot);

			place.set(layer, 0);
			return true;
		}

		return false;
	}
}
