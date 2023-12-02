#include <iostream>

#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "graphics/SpriteRenderer.h"
#include "realm/Realm.h"
#include "tileentity/CraftingStation.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/tab/CraftingTab.h"
#include "ui/tab/InventoryTab.h"
#include "util/Cast.h"

namespace Game3 {
	CraftingStation::CraftingStation(Identifier tile_id, const Position &position_, Identifier station_type, Identifier item_name):
		TileEntity(std::move(tile_id), ID(), position_, true), stationType(std::move(station_type)), itemName(std::move(item_name)) {}

	void CraftingStation::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["stationType"] = stationType;
		json["itemName"] = itemName;
	}

	bool CraftingStation::onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers modifiers, ItemStack *, Hand) {
		if (player->getSide() != Side::Server)
			return false;

		if (modifiers.onlyAlt()) {
			getRealm()->queueDestruction(getSelf());
			if (itemName)
				player->give(ItemStack(getGame(), itemName));
		} else {
			player->addStationType(stationType);
			player->queueForMove([station_type = stationType](const EntityPtr &player) {
				safeDynamicCast<Player>(player)->removeStationType(station_type);
				return true;
			});
		}

		return true;
	}

	void CraftingStation::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		stationType = json.at("stationType");
		itemName = json.at("itemName");
	}

	void CraftingStation::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << stationType;
		buffer << itemName;
	}

	void CraftingStation::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> stationType;
		buffer >> itemName;
	}
}
