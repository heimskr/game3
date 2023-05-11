#pragma once

#include "game/Game.h"

namespace Game3 {
	class ServerGame: public Game {
		public:
			std::unordered_set<PlayerPtr> players;

			using Game::Game;

			void tick();

			Side getSide() const override { return Side::Server; }
	};
}
