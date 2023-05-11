#include <iostream>

#include "Texture.h"
#include "Tileset.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "realm/Realm.h"
#include "tileentity/Chest.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/SpriteRenderer.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	Chest::Chest(Identifier tile_id, const Position &position_, std::string name_, std::shared_ptr<Texture> texture_):
	TileEntity(std::move(tile_id), ID(), position_, true), name(std::move(name_)), texture(std::move(texture_)) {
		texture->init();
	}

	void Chest::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["texture"] = *texture;
		if (inventory)
			json["inventory"] = *inventory;
		json["name"] = name;
	}

	bool Chest::onInteractNextTo(const std::shared_ptr<Player> &player) {
		if (player->getSide() == Side::Client) {
			auto &tab = *getRealm()->getGame().toClient().canvas.window.inventoryTab;
			player->queueForMove([player, &tab](const auto &) {
				tab.resetExternalInventory();
				return true;
			});
			tab.setExternalInventory(name, inventory);
		}

		return true;
	}

	void Chest::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		if (json.contains("inventory"))
			inventory = std::make_shared<Inventory>(Inventory::fromJSON(game, json.at("inventory"), shared_from_this()));
		name = json.at("name");
		texture = cacheTexture(json.at("texture"));
		texture->init();
	}

	void Chest::render(SpriteRenderer &sprite_renderer) {
		if (!isVisible())
			return;

		if (tileID) {
			// Kinda silly to get the tilesize from the realm's second layer. Maybe it could be added as a Chest field.
			auto &realm = *getRealm();
			auto &tileset = realm.getTileset();
			const auto tilesize = tileset.getTileSize();
			const auto tile_index = tileset[tileID];
			const auto x = (tile_index % (*texture->width / tilesize)) * tilesize;
			const auto y = (tile_index / (*texture->width / tilesize)) * tilesize;
			sprite_renderer(*texture, {
				.x = static_cast<float>(position.column),
				.y = static_cast<float>(position.row),
				.x_offset = x / 2.f,
				.y_offset = y / 2.f,
				.size_x = static_cast<float>(tilesize),
				.size_y = static_cast<float>(tilesize),
			});
		}
	}

	void Chest::setInventory(Slot slot_count) {
		inventory = std::make_shared<Inventory>(shared_from_this(), slot_count);
	}
}
