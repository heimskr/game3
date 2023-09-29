#pragma once

#include "Position.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "item/Item.h"
#include "realm/Realm.h"

namespace Game3 {
	template <typename T>
	class TileEntityItem: public Item {
		public:
			using Item::Item;
			bool use(Slot slot, ItemStack &stack, const Place &place, Modifiers modifiers, std::pair<float, float>) override {
				Realm &realm = *place.realm;
				Game  &game  = realm.getGame();
				assert(game.getSide() == Side::Server);

				const PlayerPtr &player   = place.player;
				const Position  &position = place.position;

				auto existing = std::dynamic_pointer_cast<T>(realm.tileEntityAt(position));
				const InventoryPtr inventory = player->getInventory();

				if (modifiers.onlyShift()) {
					if (!existing)
						return false;

					realm.queueDestruction(existing);
					inventory->add(stack.withCount(1));
					inventory->notifyOwner();
					return true;
				}

				if (!realm.isPathable(position) || realm.hasTileEntityAt(position))
					return false;

				if (std::shared_ptr<T> tile_entity = TileEntity::spawn<T>(place.realm, position)) {
					game.toServer().tileEntitySpawned(tile_entity);
					if (--stack.count == 0)
						inventory->erase(slot);
					inventory->notifyOwner();
					return true;
				}

				return false;
			}
	};
}
