#include "util/Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "packet/PlaySoundPacket.h"

namespace Game3 {
	void PlaySoundPacket::handle(const ClientGamePtr &game) {
		float volume = 1;

		if (maximumDistance != std::numeric_limits<decltype(maximumDistance)>::max()) {
			volume = 1 - game->getPlayer()->getPosition().distance(soundOrigin) / maximumDistance;
		}

		bool played = game->playSound(soundID, pitch, volume);

		if (!played) {
			WARN("Can't play unknown sound: {}", soundID);
		}
	}
}
