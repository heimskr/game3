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
	Chest::Chest(Identifier tile_id, const Position &position_, std::string name_):
		TileEntity(std::move(tile_id), ID(), position_, true), name(std::move(name_)) {}

	Chest::Chest(const Position &position_):
		Chest("base:tile/chest"_id, position_, "Chest") {}

	std::string Chest::getName() const {
		return name;
	}

	void Chest::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		const InventoryPtr inventory = getInventory();
		if (inventory)
			json["inventory"] = dynamic_cast<ServerInventory &>(*inventory);
		json["name"] = name;
	}

	bool Chest::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers) {
		if (modifiers.onlyAlt()) {
			getInventory()->iterate([&](const ItemStack &stack, Slot) {
				stack.spawn(getRealm(), getPosition());
				return false;
			});
			destroy();
			player->give(ItemStack(getGame(), "base:item/chest"));
		} else {
			addObserver(player, false);
		}

		return true;
	}

	void Chest::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		assert(getSide() == Side::Server);
		if (json.contains("inventory"))
			HasInventory::setInventory(std::make_shared<ServerInventory>(ServerInventory::fromJSON(game, json.at("inventory"), shared_from_this())));
		name = json.at("name");
	}

	void Chest::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
		buffer << name;
	}

	void Chest::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
		buffer >> name;
	}
}
