#include "Tiles.h"
#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "realm/Realm.h"
#include "tileentity/Tree.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/SpriteRenderer.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	bool Tree::onInteractNextTo(const std::shared_ptr<Player> &player) {
		auto &inventory = *player->inventory;
		const Slot active_slot = inventory.activeSlot;
		if (auto *active_stack = inventory[active_slot]) {
			if (active_stack->has(ItemAttribute::Axe)) {
				auto &realm = *getRealm();
				if (!inventory.add({Item::WOOD, 1})) {
					realm.remove(shared_from_this());
					if (active_stack->reduceDurability())
						inventory.erase(active_slot);
					ItemCount saplings = 1;
					while (rand() % 4 == 1)
						++saplings;
					auto leftover = inventory.add({Item::SAPLING, saplings});
					if (leftover)
						realm.spawn<ItemEntity>(player->position, *leftover);
					inventory.notifyOwner();
					return true;
				}
			}
		}

		return false;
	}

	void Tree::render(SpriteRenderer &sprite_renderer) {
		auto realm = getRealm();
		if (tileID != tileSets.at(realm->type)->getEmpty()) {
			auto &tilemap = *realm->tilemap2;
			const auto tilesize = tilemap.tileSize;
			const auto x = (tileID % (tilemap.setWidth / tilesize)) * tilesize;
			const auto y = (tileID / (tilemap.setWidth / tilesize)) * tilesize;
			sprite_renderer.drawOnMap(tilemap.texture, position.column, position.row, x / 2, y / 2, tilesize, tilesize);
		}
	}
}
