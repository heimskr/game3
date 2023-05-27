#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "ui/Canvas.h"
#include "ui/TextRenderer.h"

namespace Game3 {
	ClientPlayer::ClientPlayer(): Player() {}

	void ClientPlayer::render(SpriteRenderer &sprites, TextRenderer &text) {
		Player::render(sprites, text);

		text.drawOnMap(displayName, {
			.x = static_cast<float>(position.column) + offset.x() + .5f,
			.y = static_cast<float>(position.row) + offset.y(),
			.align = TextAlign::Center,
		});
	}
}
