#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "packet/ContinuousInteractionPacket.h"
#include "ui/Canvas.h"
#include "ui/TextRenderer.h"

namespace Game3 {
	ClientPlayer::ClientPlayer(): Player() {}

	std::shared_ptr<ClientPlayer> ClientPlayer::create(Game &game) {
		auto out = Entity::create<ClientPlayer>();
		out->init(game);
		return out;
	}

	void ClientPlayer::render(SpriteRenderer &sprites, TextRenderer &text) {
		Player::render(sprites, text);

		text.drawOnMap(displayName, {
			.x = static_cast<float>(position.column) + offset.x() + .5f,
			.y = static_cast<float>(position.row) + offset.y(),
			.align = TextAlign::Center,
		});
	}

	void ClientPlayer::stopContinuousInteraction() {
		send(ContinuousInteractionPacket());
	}

	void ClientPlayer::setContinuousInteraction(bool on, Modifiers modifiers) {
		if (on != continuousInteraction) {
			continuousInteraction = on;
			if (on)
				send(ContinuousInteractionPacket(modifiers));
			else
				send(ContinuousInteractionPacket());
		}

		continuousInteractionModifiers = modifiers;
	}
}
