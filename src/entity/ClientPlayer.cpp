#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "ui/Canvas.h"
#include "ui/TextRenderer.h"

namespace Game3 {
	ClientPlayer::ClientPlayer(): Player() {}

	bool thing = false;

	void ClientPlayer::render(SpriteRenderer &sprites, TextRenderer &text) {
		thing = true;
		Player::render(sprites, text);

		auto &game = getGame().toClient();
		// game.activateContext();
		text.initRenderData();
		// text.drawOnMap("a", {
		// 	.x = static_cast<float>(position.column),
		// 	.y = static_cast<float>(position.row),
		// 	// .scaleX = 10.f,
		// 	// .scaleY = 10.f,
		// });



		const auto &c = text.characters.at('a');
		GL::Texture tex(c.textureID, c.size.x, c.size.y);

		std::cout << "\e[32m" << c.textureID << "\e[39m\n";

		sprites.drawOnMap(tex, RenderOptions {
			.x = static_cast<float>(position.column),
			.y = static_cast<float>(position.row)
		});


		// drawText(game.canvas, displayName.c_str());
	}
}
