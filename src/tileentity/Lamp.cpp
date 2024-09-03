#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "graphics/SpriteRenderer.h"
#include "pipes/DataNetwork.h"
#include "realm/Realm.h"
#include "tileentity/Lamp.h"

namespace Game3 {
	namespace {
		const Identifier TILE_ID_ON = "base:tile/lamp_on";
		const Identifier TILE_ID_OFF = "base:tile/lamp_off";
	}

	Lamp::Lamp(Identifier tilename, Position position_):
		TileEntity(std::move(tilename), ID(), position_, true) {}

	Lamp::Lamp(Position position_):
		Lamp(TILE_ID_OFF, position_) {}

	void Lamp::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["on"] = on;
	}

	void Lamp::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		on = json.at("on");
		cachedTile = -1;
		cachedUpperTile = -1;
	}

	void Lamp::setOn(bool new_value) {
		if (on == new_value)
			return;
		on = new_value;
		cachedTile = -1;
		cachedUpperTile = -1;
		tileID = on? TILE_ID_ON : TILE_ID_OFF;
	}

	void Lamp::handleMessage(const AgentPtr &source, const std::string &name, std::any &data) {
		if (getSide() == Side::Server) {
			if (name == "TurnOn") {
				data = Buffer{Side::Client, on};
				if (!on) {
					setOn(true);
					queueBroadcast(true);
				}
				return;
			}

			if (name == "TurnOff") {
				data = Buffer{Side::Client, on};
				if (on) {
					setOn(false);
					queueBroadcast(true);
				}
				return;
			}

			if (name == "Toggle") {
				data = Buffer{Side::Client, on};
				setOn(!on);
				queueBroadcast(true);
				return;
			}
		}

		TileEntity::handleMessage(source, name, data);
	}

	bool Lamp::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		if (modifiers.onlyAlt()) {
			RealmPtr realm = getRealm();
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/lamp"_id));
		}

		return true;
	}

	void Lamp::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << on;
	}

	void Lamp::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		setOn(buffer.take<bool>());
	}
}
