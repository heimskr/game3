#include "graphics/Texture.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/ServerInventory.h"
#include "graphics/SpriteRenderer.h"
#include "realm/Realm.h"
#include "tileentity/Chest.h"

namespace Game3 {
	Chest::Chest(Identifier tile_id, const Position &position_, std::string name_, Identifier item_name):
		TileEntity(std::move(tile_id), ID(), position_, true), name(std::move(name_)), itemName(std::move(item_name)) {}

	Chest::Chest(const Position &position_):
		Chest("base:tile/chest"_id, position_, "Chest") {}

	std::string Chest::getName() const {
		return name;
	}

	void Chest::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		auto &object = json.as_object();
		const InventoryPtr inventory = getInventory(0);
		if (inventory) {
			object["inventory"] = boost::json::value_from(dynamic_cast<ServerInventory &>(*inventory));
		}
		object["name"] = name;
		object["itemName"] = boost::json::value_from(itemName);
	}

	bool Chest::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		assert(getSide() == Side::Server);

		if (modifiers.onlyAlt()) {
			getInventory(0)->iterate([&](const ItemStackPtr &stack, Slot) {
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

	void Chest::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		const auto &object = json.as_object();
		if (auto iter = object.find("inventory"); iter != object.end()) {
			HasInventory::setInventory(std::make_shared<ServerInventory>(boost::json::value_to<ServerInventory>(iter->value(), std::pair{game, shared_from_this()})), 0);
		}
		name = object.at("name").get_string();
		itemName = boost::json::value_to<Identifier>(object.at("itemName"));
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
