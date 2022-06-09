#include <nanogui/opengl.h>

#include <iostream>

#include "Tiles.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Realm.h"
#include "tileentity/Chest.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	Texture Chest::DEFAULT_TEXTURE {"resources/rpg/chests.png"};

	Chest::Chest(TileID id_, const Position &position_, const Texture &texture_): TileEntity(id_, TileEntity::CHEST, position_, true), texture(texture_) {
		texture.init();
	}

	void Chest::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		if (inventory)
			json["inventory"] = *inventory;
	}

	void Chest::onInteractNextTo(const std::shared_ptr<Player> &) {
		getRealm()->getGame().setText("Wow!", "Chest", true, true);
	}

	void Chest::absorbJSON(const nlohmann::json &json) {
		TileEntity::absorbJSON(json);
		if (json.contains("inventory"))
			inventory = Inventory::fromJSON(json.at("inventory"), shared_from_this());
	}

	void Chest::render(SpriteRenderer &sprite_renderer) {
		if (tileID != uint16_t(-1)) {
			// Kinda silly to get the tilesize from the realm's second layer. Maybe it could be added as a Chest field.
			auto &tilemap = *getRealm()->tilemap2;
			const auto tilesize = tilemap.tileSize;
			const auto x = (tileID % (texture.width / tilesize)) * tilesize;
			const auto y = (tileID / (texture.width / tilesize)) * tilesize;
			sprite_renderer.drawOnMap(texture, position.column, position.row, x / 2, y / 2, tilesize, tilesize);
		}
	}

	void Chest::setInventory(Slot slot_count) {
		inventory = Inventory(shared_from_this(), slot_count);
	}
}
