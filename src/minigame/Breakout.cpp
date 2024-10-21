#include "graphics/CircleRenderer.h"
#include "graphics/Color.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "minigame/Breakout.h"
#include "ui/gl/UIContext.h"
#include "util/Math.h"

namespace Game3 {
	namespace {
		constexpr Color BREAKOUT_FOREGROUND{"#425229"};
		constexpr Color BREAKOUT_BACKGROUND{"#ffffce"};
		constexpr double BREAKOUT_CHECK_TIME = 0.05;
		constexpr std::size_t BREAKOUT_BLOCK_SCORE = 500;
		bool checkCircleRectangleIntersection(Vector2i circle, int radius, const Rectangle &);
	}

	Breakout::Breakout():
		ballSize(20),
		blockWidth(50),
		blockHeight(20),
		blockPadding(10),
		paddleWidth(100),
		paddleHeight(30),
		rowCount(10) {}

	void Breakout::tick(double delta) {
		accumulatedTime += delta;
		if (accumulatedTime < BREAKOUT_CHECK_TIME) {
			return;
		}

		accumulatedTime = 0;
		auto iter = getBlockIntersection(ballPosition);
		if (iter != blocks.end()) {
			breakBlock(iter);
		}
	}

	void Breakout::render(UIContext &ui, const RendererContext &renderers) {
		RectangleRenderer &rectangler = renderers.rectangle;

		auto [mx, my] = ui.getMouseCoordinates();
		auto [sx, sy, sw, sh] = ui.scissorStack.getTop().rectangle;
		mx -= sx;
		my -= sy;

		ballPosition = Vector2i(mx, my);

		rectangler.drawOnScreen(BREAKOUT_BACKGROUND, 0, 0, gameWidth, gameHeight);

		for (const Rectangle &block: blocks) {
			rectangler.drawOnScreen(BREAKOUT_FOREGROUND, block);
		}

		rectangler.drawOnScreen(BREAKOUT_FOREGROUND, paddle);

		renderers.circle.drawOnScreen(checkCircleRectangleIntersection(ballPosition, ballSize, paddle)? Color{"#00ff00"} : Color{"#ff0000"}, ballPosition.x, ballPosition.y, ballSize, ballSize);
	}

	void Breakout::setSize(float width, float height) {
		gameWidth = width;
		gameHeight = height;
	}

	void Breakout::reset() {
		blocks.clear();

		const int count_x = (gameWidth - blockPadding) / (blockPadding + blockWidth);
		const int pad_x = (gameWidth - count_x * (blockWidth + blockPadding) + blockPadding) / 2;

		for (int row = 0; row < rowCount; ++row) {
			const int top = row * (blockHeight + blockPadding) + blockPadding;
			for (int left = pad_x; left + blockWidth + blockPadding <= gameWidth; left += blockWidth + blockPadding) {
				blocks.emplace_back(left, top, blockWidth, blockHeight);
			}
		}

		ballPosition = {ballSize, rowCount * (blockHeight + blockPadding) + blockPadding + ballSize};
		ballVelocity = {1, 1};
		ballSpeed = 2;
		paddle = {blockPadding, gameHeight - blockPadding - paddleHeight, paddleWidth, paddleHeight};
		normalizeVelocity();
	}

	std::list<Rectangle>::iterator Breakout::getBlockIntersection(Vector2i position) {
		for (const Rectangle &block: blocks) {

		}

		return blocks.end();
	}

	void Breakout::normalizeVelocity() {
		double current_speed = std::sqrt(static_cast<double>(ballVelocity.x * ballVelocity.x + ballVelocity.y * ballVelocity.y));
		double factor = ballSpeed / current_speed;
		if (factor == 1) {
			return;
		}

		ballVelocity.x *= factor;
		ballVelocity.y *= factor;
	}

	void Breakout::breakBlock(std::list<Rectangle>::iterator iter) {
		blocks.erase(iter);
		increaseScore(BREAKOUT_BLOCK_SCORE);
	}

	namespace {
		bool checkCircleRectangleIntersection(Vector2i circle, int radius, const Rectangle &rectangle) {
			// https://stackoverflow.com/a/402010

			const Vector2i circle_distance{
				std::abs(circle.x - (rectangle.x + rectangle.width / 2)),
				std::abs(circle.y - (rectangle.y + rectangle.height / 2)),
			};

			if (circle_distance.x > rectangle.width / 2 + radius || circle_distance.y > rectangle.height / 2 + radius) {
				return false;
			}

			if (circle_distance.x <= rectangle.width / 2 || circle_distance.y <= rectangle.height / 2) {
				return true;
			}

			return sqr(circle_distance.x - rectangle.width / 2) + sqr(circle_distance.y - rectangle.height / 2) <= sqr(radius);
		}
	}
}
