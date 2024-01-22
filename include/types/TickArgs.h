#pragma once

#include "types/Types.h"

namespace Game3 {
	class Game;

	struct TickArgs {
		Game &game;
		Tick tick;
		float delta;

		TickArgs(Game &game_, Tick tick_, float delta_):
			game(game_), tick(tick_), delta(delta_) {}
	};
}
