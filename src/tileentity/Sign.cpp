#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/ServerGame.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "lib/JSON.h"
#include "packet/InteractPacket.h"
#include "packet/UpdateAgentFieldPacket.h"
#include "realm/Realm.h"
#include "tileentity/Sign.h"
#include "ui/Window.h"
#include "ui/gl/dialog/EditSignDialog.h"
#include "util/ConstexprHash.h"

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
		if (player->getSide() == Side::Server) {
			player->send(make<InteractPacket>(false, hand, modifiers, getGID()));
		} else if (!modifiers.onlyShift()) {
			player->showText(text, name);
		} else {
			ClientGamePtr game = player->getGame()->toClientPointer();
			WindowPtr window = game->getWindow();
			window->queue([this, game](Window &window) {
				auto dialog = window.uiContext.emplaceDialog<EditSignDialog>(1.f, 1200, 400, tileID.str(), text);
				dialog->onSubmit.connect([this, game](UString tilename, UString contents) {
					game->send(make<UpdateAgentFieldPacket>(*this, "tileID"_fnv, tilename));
					game->send(make<UpdateAgentFieldPacket>(*this, "text"_fnv, contents));
				});
			});
		}
		return true;
	}

	void Sign::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		text = json.at("text").as_string();
		name = json.at("name").as_string();
	}

	bool Sign::setField(uint32_t field_name, Buffer &field_value, const PlayerPtr &updater) {
		switch (field_name) {
			AGENT_FIELD_CUSTOM(tileID, true, resetTileCache());
			AGENT_FIELD(text, true);
			AGENT_FIELD(name, true);
			default:
				return TileEntity::setField(field_name, field_value, updater);
		}
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
