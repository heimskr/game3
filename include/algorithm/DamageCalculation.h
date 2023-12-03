#pragma once

#include "types/Types.h"

namespace Game3 {
	HitPoints calculateDamage(HitPoints weapon_damage, int variability, double attacker_luck);
}
