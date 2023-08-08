#include <iostream>

#include "Texture.h"
#include "Tileset.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/ServerInventory.h"
#include "realm/Realm.h"
#include "tileentity/Chest.h"
#include "ui/SpriteRenderer.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	Chest::Chest(Identifier tile_id, const Position &position_, std::string name_, std::shared_ptr<Texture> texture_):
	TileEntity(std::move(tile_id), ID(), position_, true), name(std::move(name_)), texture(std::move(texture_)) {
		if (texture)
			texture->init();
	}

	void Chest::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["texture"] = *texture;
		if (inventory)
			json["inventory"] = dynamic_cast<ServerInventory &>(*inventory);
		json["name"] = name;
	}

	bool Chest::onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers) {
		addObserver(player);
		return true;
	}

	void Chest::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		if (json.contains("inventory"))
			inventory = std::make_shared<ServerInventory>(ServerInventory::fromJSON(game, json.at("inventory"), shared_from_this()));
		name = json.at("name");
		texture = cacheTexture(json.at("texture"));
		texture->init();
	}

	void Chest::render(SpriteRenderer &sprite_renderer) {
		if (!isVisible() || !tileID)
			return;

		auto &tileset = getRealm()->getTileset();
		const auto tilesize = tileset.getTileSize();
		const auto tile_index = tileset[tileID];
		assert(texture);
		texture->init();
		assert(texture->width);
		const auto x = (tile_index % (texture->width / tilesize)) * tilesize;
		const auto y = (tile_index / (texture->width / tilesize)) * tilesize;
		sprite_renderer(*texture, {
			.x = float(position.column),
			.y = float(position.row),
			.x_offset = float(x) / 2.f,
			.y_offset = float(y) / 2.f,
			.size_x = float(tilesize),
			.size_y = float(tilesize),
		});
	}

	void Chest::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
		buffer << name;
		buffer << texture->path.string();
	}

	void Chest::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
		buffer >> name;
		const std::filesystem::path texture_path = buffer.take<std::string>();
		texture = cacheTexture(texture_path);
	}
}
