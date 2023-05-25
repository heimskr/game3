#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "ui/Canvas.h"
#include "ui/TextRenderer.h"

namespace Game3 {
	ClientPlayer::ClientPlayer(): Player() {}

	void ClientPlayer::render(SpriteRenderer &sprites, TextRenderer &text) {
		Player::render(sprites, text);

		auto &game = getGame().toClient();
		text.drawOnMap(displayName, {.x = static_cast<float>(position.column), .y = static_cast<float>(position.row)});
		// drawText(game.canvas, displayName.c_str());
	}
}
