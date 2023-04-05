#include <iostream>

#include "Tiles.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/CraftingStation.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/SpriteRenderer.h"
#include "ui/tab/CraftingTab.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	CraftingStation::CraftingStation(TileID id_, const Position &position_, CraftingStationType station_type):
		TileEntity(id_, TileEntity::CRAFTING_STATION, position_, true), stationType(station_type) {}

	void CraftingStation::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["stationType"] = stationType;
	}

	bool CraftingStation::onInteractNextTo(const std::shared_ptr<Player> &player) {
		player->stationTypes.insert(stationType);
		auto &tab = *getRealm()->getGame().canvas.window.craftingTab;
		tab.reset(player->getRealm()->getGame().shared_from_this());
		tab.show();
		player->queueForMove([player, station_type = stationType, &tab](const auto &) {
			player->stationTypes.erase(station_type);
			tab.reset(player->getRealm()->getGame().shared_from_this());
			player->getRealm()->getGame().canvas.window.inventoryTab->show();
			return true;
		});
		return true;
	}

	void CraftingStation::absorbJSON(const nlohmann::json &json) {
		TileEntity::absorbJSON(json);
		stationType = json.at("stationType");
	}
}
