#pragma once

#include "graphics/Rectangle.h"
#include "minigame/Minigame.h"
#include "types/Position.h"

#include <list>

namespace Game3 {
	class Breakout: public Minigame {
		public:
			static Identifier ID() { return {"base", "minigame/breakout"}; }
			std::string getName() const final { return "Breakout"; }

			Breakout();

			void tick(UIContext &, double delta) final;
			void render(UIContext &, const RendererContext &) final;
			void setSize(int width, int height) final;
			void reset() final;

		private:
			int lives{};
			int ballSize{};
			int blockWidth{};
			int blockHeight{};
			int blockPadding{};
			int paddleWidth{};
			int paddleHeight{};
			int paddleSpeed{};
			int rowCount{};
			int blocksBottom{};
			Vector2d ballPosition{};
			Vector2d ballVelocity{};
			Rectangle paddle{};
			double ballSpeed{};
			double accumulatedTime{};
			std::list<Rectangle> blocks;
			bool isGameOver = false;

			std::list<Rectangle>::iterator getBlockIntersection();
			void normalizeVelocity();
			void breakBlock(std::list<Rectangle>::iterator);
			void ballLost();
			void bounceX();
			void bounceY();
			void resetBallPosition();
			void gameOver();
	};
}
