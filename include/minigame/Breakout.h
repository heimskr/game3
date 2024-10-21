#pragma once

#include "graphics/Rectangle.h"
#include "minigame/Minigame.h"
#include "types/Position.h"

#include <list>

namespace Game3 {
	class Breakout: public Minigame {
		public:
			Breakout();

			void tick(double delta) final;
			void render(UIContext &, const RendererContext &) final;
			void setSize(float width, float height) final;
			void reset();

			int gameWidth{};
			int gameHeight{};

		private:
			int ballSize{};
			int blockWidth{};
			int blockHeight{};
			int blockPadding{};
			int paddleWidth{};
			int paddleHeight{};
			int rowCount{};
			Vector2i ballPosition{};
			Vector2i ballVelocity{};
			Rectangle paddle{};
			double ballSpeed{};
			double accumulatedTime{};
			std::list<Rectangle> blocks;

			std::list<Rectangle>::iterator getBlockIntersection(Vector2i position);
			void normalizeVelocity();
			void breakBlock(std::list<Rectangle>::iterator);
	};
}
