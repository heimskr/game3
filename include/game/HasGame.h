#pragma once

#include <memory>

namespace Game3 {
	class Game;

	class HasGame {
		public:
			virtual ~HasGame() = default;

			virtual std::shared_ptr<Game> getGame() const {
				if (auto game = weakGame.lock())
					return game;
				throw std::runtime_error("Couldn't lock game");
			}

			std::shared_ptr<Game> tryGame() const {
				return weakGame.lock();
			}

			void setGame(const std::shared_ptr<Game> &game) {
				weakGame = game;
			}

		protected:
			HasGame() = default;
			HasGame(const std::shared_ptr<Game> &game):
				weakGame(game) {}

		private:
			std::weak_ptr<Game> weakGame;
	};
}
