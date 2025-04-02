#include "entity/Player.h"
#include "game/ClientGame.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "lib/JSON.h"
#include "packet/DisplayTextPacket.h"
#include "realm/Realm.h"
#include "tileentity/Sign.h"
#include "ui/Window.h"

namespace Game3 {
	Sign::Sign(Identifier tilename, Position position_, std::string text_, std::string name_):
		TileEntity(std::move(tilename), ID(), position_, false), text(std::move(text_)), name(std::move(name_)) {}

	void Sign::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		auto &object = ensureObject(json);
		object["text"] = text;
		object["name"] = name;
	}

	bool Sign::onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers, const ItemStackPtr &, Hand) {
		player->showText(text, name);
		return true;
	}

	void Sign::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		text = json.at("text").as_string();
		name = json.at("name").as_string();
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
