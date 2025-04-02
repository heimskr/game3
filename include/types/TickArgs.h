#pragma once

#include "types/Types.h"

namespace Game3 {
	class Game;

	struct TickArgs {
		std::shared_ptr<Game> game;
		Tick tick;
		float delta;

		TickArgs(std::shared_ptr<Game> game, Tick tick, float delta):
			game(std::move(game)), tick(tick), delta(delta) {}
	};
}
