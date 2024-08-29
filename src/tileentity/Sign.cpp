#include <iostream>

#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "graphics/SpriteRenderer.h"
#include "packet/DisplayTextPacket.h"
#include "realm/Realm.h"
#include "tileentity/Sign.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"

namespace Game3 {
	Sign::Sign(Identifier tilename, Position position_, std::string text_, std::string name_):
		TileEntity(std::move(tilename), ID(), position_, false), text(std::move(text_)), name(std::move(name_)) {}

	void Sign::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["text"] = text;
		json["name"] = name;
	}

	bool Sign::onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers, const ItemStackPtr &, Hand) {
		player->showText(text, name);
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
