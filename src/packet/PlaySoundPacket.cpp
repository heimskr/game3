#include "util/Log.h"
#include "entity/Entity.h"
#include "game/ClientGame.h"
#include "packet/PlaySoundPacket.h"

namespace Game3 {
	void PlaySoundPacket::handle(const ClientGamePtr &game) {
		// TODO: origin

		bool played = game->playSound(soundID, pitch);

		if (!played)
			WARN("Can't play unknown sound: {}", soundID);
	}
}
