#include <iostream>

#include "graphics/Texture.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/ServerInventory.h"
#include "graphics/SpriteRenderer.h"
#include "realm/Realm.h"
#include "tileentity/Chest.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	Chest::Chest(Identifier tile_id, const Position &position_, std::string name_, Identifier item_name):
		TileEntity(std::move(tile_id), ID(), position_, true), name(std::move(name_)), itemName(std::move(item_name)) {}

	Chest::Chest(const Position &position_):
		Chest("base:tile/chest"_id, position_, "Chest") {}

	std::string Chest::getName() const {
		return name;
	}

	void Chest::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		const InventoryPtr inventory = getInventory(0);
		if (inventory)
			json["inventory"] = dynamic_cast<ServerInventory &>(*inventory);
		json["name"] = name;
		json["itemName"] = itemName;
	}

	bool Chest::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers) {
		assert(getSide() == Side::Server);

		if (modifiers.onlyAlt()) {
			getInventory(0)->iterate([&](const ItemStack &stack, Slot) {
				stack.spawn(getRealm(), getPosition());
				return false;
			});
			destroy();
			player->give(ItemStack(getGame(), itemName));
		} else {
			addObserver(player, false);
		}

		return true;
	}

	void Chest::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		assert(getSide() == Side::Server);
		if (json.contains("inventory"))
			HasInventory::setInventory(std::make_shared<ServerInventory>(ServerInventory::fromJSON(game, json.at("inventory"), shared_from_this())), 0);
		name = json.at("name");
		itemName = json.at("itemName");
	}

	void Chest::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
		buffer << name;
		buffer << itemName;
	}

	void Chest::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
		buffer >> name;
		buffer >> itemName;
	}
}
