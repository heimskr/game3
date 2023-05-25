#include "entity/ClientPlayer.h"
#include "ui/Canvas.h"
#include "ui/SpriteRenderer.h"
#include "ui/Text.h"

namespace Game3 {
	ClientPlayer::ClientPlayer():
	Player(), label(std::make_unique<FTLabel>(uiFont, "", 0, 0, 0, 0)) {
		// label->setAlignment(FTLabel::FontFlags::CenterAligned);
		label->setPixelSize(48);
		label->setColor(0.f, 0.f, 0.f, 1.f);
	}

	void ClientPlayer::render(SpriteRenderer &renderer) {
		Player::render(renderer);

		if (renderer.backbufferWidth != backbufferWidth || renderer.backbufferHeight != backbufferHeight) {
			backbufferWidth  = renderer.backbufferWidth;
			backbufferHeight = renderer.backbufferHeight;
			label->setWindowSize(backbufferWidth, backbufferHeight);
		}

		label->setText("What the fuck is this shit?");
		label->render();
	}
}
