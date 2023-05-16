#include <iostream>

#include "Tileset.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
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
			auto &game = getRealm()->getGame().toClient();
			auto &tab = *game.canvas.window.craftingTab;
			tab.reset(game.toClientPointer());
			tab.show();
			player->queueForMove([&game, player, station_type = stationType, &tab](const auto &) {
				player->stationTypes.erase(station_type);
				tab.reset(game.toClientPointer());
				game.getWindow().inventoryTab->show();
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

	void CraftingStation::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << stationType;
	}

	void CraftingStation::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> stationType;
	}
}
