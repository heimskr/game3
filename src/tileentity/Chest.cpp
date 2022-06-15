#include <iostream>

#include "Tiles.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Chest.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/SpriteRenderer.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	Texture Chest::DEFAULT_TEXTURE {"resources/rpg/chests.png"};

	Chest::Chest(TileID id_, const Position &position_, const std::string &name_, const Texture &texture_): TileEntity(id_, TileEntity::CHEST, position_, true), name(name_), texture(texture_) {
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

	void Chest::absorbJSON(const nlohmann::json &json) {
		TileEntity::absorbJSON(json);
		if (json.contains("inventory"))
			inventory = std::make_shared<Inventory>(Inventory::fromJSON(json.at("inventory"), shared_from_this()));
		name = json.at("name");
		texture = json.at("texture");
		texture.init();
	}

	void Chest::render(SpriteRenderer &sprite_renderer) {
		if (tileID != uint16_t(-1)) {
			// Kinda silly to get the tilesize from the realm's second layer. Maybe it could be added as a Chest field.
			auto &tilemap = *getRealm()->tilemap2;
			const auto tilesize = tilemap.tileSize;
			const auto x = (tileID % (texture.width / tilesize)) * tilesize;
			const auto y = (tileID / (texture.width / tilesize)) * tilesize;
			sprite_renderer.drawOnMap(texture, position.column, position.row, x / 2.f, y / 2.f, tilesize, tilesize);
		}
	}

	void Chest::setInventory(Slot slot_count) {
		inventory = std::make_shared<Inventory>(shared_from_this(), slot_count);
	}
}
