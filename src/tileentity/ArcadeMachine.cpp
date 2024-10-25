#include <iostream>

#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "graphics/SpriteRenderer.h"
#include "minigame/MinigameFactory.h"
#include "packet/OpenMinigamePacket.h"
#include "realm/Realm.h"
#include "tileentity/ArcadeMachine.h"
#include "ui/gl/dialog/MinigameDialog.h"
#include "ui/Window.h"

namespace Game3 {
	ArcadeMachine::ArcadeMachine(Identifier tilename, Position position, Identifier minigame_name, int game_width, int game_height):
		TileEntity(std::move(tilename), ID(), position, true),
		minigameName(std::move(minigame_name)),
		gameWidth(game_width),
		gameHeight(game_height) {}

	bool ArcadeMachine::onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers, const ItemStackPtr &, Hand) {
		player->send(make<OpenMinigamePacket>(minigameName, gameWidth, gameHeight));
		return true;
	}

	void ArcadeMachine::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << minigameName;
		buffer << gameWidth;
		buffer << gameHeight;
	}

	void ArcadeMachine::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> minigameName;
		buffer >> gameWidth;
		buffer >> gameHeight;
	}
}
