#include <iostream>

#include "graphics/Texture.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "game/ExpandedServerInventory.h"
#include "game/ServerInventory.h"
#include "graphics/SpriteRenderer.h"
#include "realm/Realm.h"
#include "tileentity/Crate.h"

namespace Game3 {
	Crate::Crate(Identifier tile_id, const Position &position_, Identifier item_name, std::string name_):
		TileEntity(std::move(tile_id), ID(), position_, true), itemName(std::move(item_name)), name(std::move(name_)) {}

	Crate::Crate(const Position &position_):
		Crate("base:tile/crate"_id, position_, "base:item/crate", "Crate") {}

	std::string Crate::getName() const {
		return name;
	}

	void Crate::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["name"] = name;
		json["itemName"] = itemName;
		if (InventoryPtr inventory = getInventory(0))
			json["inventory"] = dynamic_cast<ExpandedServerInventory &>(*inventory);
	}

	bool Crate::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		assert(getSide() == Side::Server);

		if (modifiers.onlyAlt()) {
			getInventory(0)->iterate([this](const ItemStackPtr &stack, Slot) {
				stack->spawn(getPlace());
				return false;
			});
			queueDestruction();
			player->give(ItemStack::create(getGame(), itemName));
		} else {
			addObserver(player, false);
		}

		return true;
	}

	void Crate::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		assert(game->getSide() == Side::Server);
		name = json.at("name");
		itemName = json.at("itemName");
		if (auto iter = json.find("inventory"); iter != json.end())
			HasInventory::setInventory(std::make_shared<ExpandedServerInventory>(ServerInventory::fromJSON(game, *iter, shared_from_this())), 0);
	}

	void Crate::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
		buffer << itemName;
		buffer << name;
	}

	void Crate::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		setInventory(1);
		if (getSharedAgent()->getSide() == Side::Client)
			decodeSpecific<ClientInventory>(buffer, 0);
		else
			decodeSpecific<ExpandedServerInventory>(buffer, 0);
		buffer >> itemName;
		buffer >> name;
	}

	ItemCount Crate::itemsInsertable(const ItemStackPtr &stack, Direction, Slot) {
		InventoryPtr inventory = getInventory(0);

		ItemStackPtr stored = (*inventory)[0];
		if (!stored)
			return stack->count;

		return stored->canMerge(*stack)? stack->count : 0;
	}

	void Crate::setInventory(Slot slot_count) {
		if (getSide() == Side::Client)
			HasInventory::setInventory(std::make_shared<ClientInventory>(shared_from_this(), slot_count), 0);
		else
			HasInventory::setInventory(std::make_shared<ExpandedServerInventory>(shared_from_this(), slot_count), 0);
		inventoryUpdated(0);
	}
}
