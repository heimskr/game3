#include "entity/Player.h"
#include "game/ClientGame.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "lib/JSON.h"
#include "packet/InteractPacket.h"
#include "realm/Realm.h"
#include "tileentity/Sign.h"
#include "ui/Window.h"
#include "ui/gl/dialog/EditSignDialog.h"

namespace Game3 {
	Sign::Sign(Identifier tilename, Position position_, std::string text_, std::string name_):
		TileEntity(std::move(tilename), ID(), position_, false), text(std::move(text_)), name(std::move(name_)) {}

	void Sign::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		auto &object = ensureObject(json);
		object["text"] = text;
		object["name"] = name;
	}

	bool Sign::onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers modifiers, const ItemStackPtr &, Hand hand) {
		INFO("Sign side: {}", player->getSide());
		if (player->getSide() == Side::Server) {
			player->send(make<InteractPacket>(false, hand, modifiers, getGID()));
		} else {
			// player->showText(text, name);
			WindowPtr window = player->getGame()->toClient().getWindow();
			window->queue([this](Window &window) {
				window.uiContext.emplaceDialog<EditSignDialog>(1.f, 800, 400, tileID.str(), text);
			});
		}
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
