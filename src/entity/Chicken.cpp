#include "entity/Chicken.h"

namespace Game3 {
	void Chicken::tick(Game &game, float delta) {
		timeUntilEgg -= delta;

		if (timeUntilEgg <= 0.f) {
			timeUntilEgg = EGG_PERIOD;
			if (getSide() == Side::Server)
				ItemStack(game, "base:item/egg").spawn(getRealm(), position);
		}

		Animal::tick(game, delta);
	}

	void Chicken::encode(Buffer &buffer) {
		Animal::encode(buffer);
		buffer << timeUntilEgg;
	}

	void Chicken::decode(Buffer &buffer) {
		Animal::decode(buffer);
		buffer >> timeUntilEgg;
	}
}
