#include "entity/Player.h"
#include "game/Game.h"
#include "graphics/Tileset.h"
#include "item/VoidPickaxe.h"
#include "realm/Realm.h"
#include "types/Position.h"
#include "util/Reverse.h"

namespace Game3 {
	bool VoidPickaxe::use(Slot slot, const ItemStackPtr &, const Place &place, Modifiers modifiers, std::pair<float, float>) {
		PlayerPtr player = place.player;
		RealmPtr realm = place.realm;
		GamePtr game = realm->getGame();

		if (modifiers.onlyCtrl()) {
			if (TileEntityPtr tile_entity = realm->tileEntityAt(place.position)) {
				tile_entity->queueDestruction();
				return true;
			}

			return false;
		}

		Tileset &tileset = realm->getTileset();

		for (const Layer layer: reverse(allLayers)) {
			std::optional<TileID> tile = place.get(layer);
			if (!tile || *tile == 0) {
				continue;
			}

			ItemStackPtr stack;
			if (tileset.getItemStack(game, tileset[*tile], stack)) {
				player->give(stack, slot);
			}

			place.set(layer, 0);
			return true;
		}

		return false;
	}

	bool VoidPickaxe::drag(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets, DragAction) {
		if (modifiers.onlyShift()) {
			return use(slot, stack, place, modifiers, offsets);
		}
		return false;
	}
}
