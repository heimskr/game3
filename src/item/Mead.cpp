#include "item/Mead.h"

namespace Game3 {
	HitPoints Mead::getHealedPoints(const PlayerPtr &player) {
		return (player->getMaxHealth() - player->getHealth()) / 4;
	}
}
