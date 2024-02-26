#pragma once

#include "types/Types.h"

namespace Game3 {
	class Game;

	struct TickArgs {
		std::shared_ptr<Game> game;
		Tick tick;
		float delta;

		TickArgs(std::shared_ptr<Game> game_, Tick tick_, float delta_):
			game(std::move(game_)), tick(tick_), delta(delta_) {}
	};
}
