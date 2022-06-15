#include "Tiles.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "realm/Realm.h"
#include "tileentity/Tree.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	bool Tree::onInteractNextTo(const std::shared_ptr<Player> &player) {
		auto &inventory = *player->inventory;

		if (auto slot = inventory.find(ItemAttribute::Axe)) {
			if (!inventory.add({Item::WOOD, 1}))
				getRealm()->remove(shared_from_this());
			if (inventory[*slot]->reduceDurability())
				inventory.erase(*slot);
			return true;
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
