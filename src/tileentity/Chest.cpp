#include <iostream>

#include "Texture.h"
#include "Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Chest.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/SpriteRenderer.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	Texture Chest::DEFAULT_TEXTURE = cacheTexture("resources/rpg/chests.png");

	Chest::Chest(Identifier tile_id, const Position &position_, std::string name_, Texture texture_):
	TileEntity(std::move(tile_id), ID(), position_, true), name(std::move(name_)), texture(std::move(texture_)) {
		texture.init();
	}

	void Chest::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["texture"] = texture;
		if (inventory)
			json["inventory"] = *inventory;
		json["name"] = name;
	}

	bool Chest::onInteractNextTo(const std::shared_ptr<Player> &player) {
		auto &tab = *getRealm()->getGame().canvas.window.inventoryTab;
		player->queueForMove([player, &tab](const auto &) {
			tab.resetExternalInventory();
			return true;
		});
		tab.setExternalInventory(name, inventory);
		return true;
	}

	void Chest::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		if (json.contains("inventory"))
			inventory = std::make_shared<Inventory>(Inventory::fromJSON(json.at("inventory"), shared_from_this()));
		name = json.at("name");
		texture = json.at("texture");
		texture.init();
	}

	void Chest::render(SpriteRenderer &sprite_renderer) {
		if (!isVisible())
			return;

		if (tileID != uint16_t(-1)) {
			// Kinda silly to get the tilesize from the realm's second layer. Maybe it could be added as a Chest field.
			auto &tilemap = *getRealm()->tilemap2;
			const auto tilesize = tilemap.tileSize;
			const auto x = (tileID % (*texture.width / tilesize)) * tilesize;
			const auto y = (tileID / (*texture.width / tilesize)) * tilesize;
			sprite_renderer.drawOnMap(texture, position.column, position.row, x / 2.f, y / 2.f, tilesize, tilesize);
		}
	}

	void Chest::setInventory(Slot slot_count) {
		inventory = std::make_shared<Inventory>(shared_from_this(), slot_count);
	}
}
