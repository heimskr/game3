#include <iostream>

#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "graphics/SpriteRenderer.h"
#include "packet/OpenTextTabPacket.h"
#include "realm/Realm.h"
#include "tileentity/Sign.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/tab/TextTab.h"

namespace Game3 {
	Sign::Sign(Identifier tilename, Position position_, std::string text_, std::string name_):
		TileEntity(std::move(tilename), ID(), position_, false), text(std::move(text_)), name(std::move(name_)) {}

	void Sign::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["text"] = text;
		json["name"] = name;
	}

	bool Sign::onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers, ItemStack *, Hand) {
		player->send(OpenTextTabPacket(name, text, true, true));
		return true;
	}

	void Sign::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		text = json.at("text");
		name = json.at("name");
	}

	void Sign::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << text;
		buffer << name;
	}

	void Sign::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> text;
		buffer >> name;
	}
}
