#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "ui/Canvas.h"
#include "ui/SpriteRenderer.h"
#include "ui/Text.h"

namespace Game3 {
	ClientPlayer::ClientPlayer(): Player() {}

	void ClientPlayer::render(SpriteRenderer &renderer) {
		Player::render(renderer);

		getGame().toClient().activateContext();
		drawText(displayName.c_str());
	}
}
