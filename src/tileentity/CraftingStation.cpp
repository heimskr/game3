#include <iostream>

#include "Tileset.h"
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
	CraftingStation::CraftingStation(Identifier tile_id, const Position &position_, Identifier station_type):
		TileEntity(std::move(tile_id), ID(), position_, true), stationType(std::move(station_type)) {}

	void CraftingStation::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["stationType"] = stationType;
	}

	bool CraftingStation::onInteractNextTo(const std::shared_ptr<Player> &player) {
		player->stationTypes.insert(stationType);

		if (player->getSide() == Side::Client) {
			auto &tab = *getRealm()->getGame().canvas->window.craftingTab;
			tab.reset(player->getRealm()->getGame().shared_from_this());
			tab.show();
			player->queueForMove([player, station_type = stationType, &tab](const auto &) {
				player->stationTypes.erase(station_type);
				tab.reset(player->getRealm()->getGame().shared_from_this());
				player->getRealm()->getGame().canvas->window.inventoryTab->show();
				return true;
			});
		} else {
			player->queueForMove([player, station_type = stationType](const auto &) {
				player->stationTypes.erase(station_type);
				return true;
			});
		}

		return true;
	}

	void CraftingStation::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		stationType = json.at("stationType");
	}
}
