#include "graphics/CircleRenderer.h"
#include "graphics/Color.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "minigame/Breakout.h"
#include "ui/gl/UIContext.h"
#include "ui/Window.h"
#include "util/Math.h"

namespace Game3 {
	namespace {
		constexpr Color BREAKOUT_FOREGROUND{"#425229"};
		constexpr Color BREAKOUT_BACKGROUND{"#ffffce"};
		constexpr double BREAKOUT_CHECK_TIME = 0.05;
		constexpr std::size_t BREAKOUT_BLOCK_SCORE = 500;

		template <typename V>
		bool checkCircleRectangleIntersection(const V &circle, int radius, const Rectangle &rectangle) {
			// https://stackoverflow.com/a/402010

			const Vector2d circle_distance{
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

	Breakout::Breakout():
		ballSize(20),
		blockWidth(50),
		blockHeight(20),
		blockPadding(10),
		paddleWidth(160),
		paddleHeight(30),
		paddleSpeed(8),
		rowCount(10) {}

	void Breakout::tick(UIContext &ui, double delta) {
		accumulatedTime += delta;
		if (accumulatedTime < BREAKOUT_CHECK_TIME) {
			return;
		}

		accumulatedTime = 0;

		ballPosition.x += ballVelocity.x;
		ballPosition.y += ballVelocity.y;

		if (checkCircleRectangleIntersection(ballPosition, ballSize, paddle)) {
			// if (ballPosition.y + ballSize <= paddle.y) {
			if (paddle.contains(ballPosition.x, ballPosition.y + ballSize)) {
				// https://gamedev.stackexchange.com/a/21048
				bounceY();
				ballPosition.y = paddle.y - ballSize - 1;
				const int paddle_center = paddle.x + paddle.width / 2;
				const double pos_x = (ballPosition.x - paddle_center) / (paddle.width / 2.0);
				constexpr double influence = 0.9;
				ballVelocity.x = ballSpeed * pos_x * influence;
				normalizeVelocity();
			}
		}

		if (ballPosition.x + ballSize >= gameWidth) {
			ballPosition.x = gameWidth - ballSize;
			bounceX();
		} else if (ballPosition.x - ballSize <= 0) {
			ballPosition.x = ballSize;
			bounceX();
		}

		if (ballPosition.y - ballSize >= gameHeight) {
			ballLost();
		} else if (ballPosition.y - ballSize <= 0) {
			ballPosition.y = ballSize;
			bounceY();
		}

		const decltype(ballPosition) top    = {ballPosition.x, ballPosition.y - ballSize};
		const decltype(ballPosition) bottom = {ballPosition.x, ballPosition.y + ballSize};
		const decltype(ballPosition) left   = {ballPosition.x - ballSize, ballPosition.y};
		const decltype(ballPosition) right  = {ballPosition.x + ballSize, ballPosition.y};

		const bool left_held = ui.window.isKeyHeld(GLFW_KEY_LEFT);
		const bool right_held = ui.window.isKeyHeld(GLFW_KEY_RIGHT);

		if (left_held) {
			Rectangle new_paddle = paddle;
			new_paddle.x = std::max(0, new_paddle.x - paddleSpeed);
			if (new_paddle.contains(right.x, right.y)) {
				bounceX();
				new_paddle.x = right.x;
			}
			paddle = new_paddle;
		} else if (right_held) {
			Rectangle new_paddle = paddle;
			new_paddle.x = std::min(gameWidth - paddle.width, new_paddle.x + paddleSpeed);
			if (new_paddle.contains(left.x, left.y)) {
				bounceX();
				new_paddle.x = left.x - paddle.width;
			}
			paddle = new_paddle;
		}

		auto iter = getBlockIntersection();
		if (iter != blocks.end()) {
			Rectangle block = *iter;

			bool bounced = false;

			if (block.contains(left.x, left.y) || block.contains(right.x, right.y)) {
				bounceX();
				bounced = true;
			}

			if (block.contains(top.x, top.y) || block.contains(bottom.x, bottom.y)) {
				bounceY();
				bounced = true;
			}

			if (bounced) {
				breakBlock(iter);
			}
		}
	}

	void Breakout::render(UIContext &ui, const RendererContext &renderers) {
		RectangleRenderer &rectangler = renderers.rectangle;

		rectangler.drawOnScreen(BREAKOUT_BACKGROUND, 0, 0, gameWidth, gameHeight);

		for (const Rectangle &block: blocks) {
			rectangler.drawOnScreen(BREAKOUT_FOREGROUND, block);
		}

		rectangler.drawOnScreen(BREAKOUT_FOREGROUND, paddle);

		renderers.circle.drawOnScreen(BREAKOUT_FOREGROUND, ballPosition.x, ballPosition.y, ballSize, ballSize);
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

		blocksBottom = rowCount * (blockHeight + blockPadding) + blockPadding + ballSize;
		ballPosition.x = ballSize;
		ballPosition.y = blocksBottom;
		ballVelocity = {1, 1};
		ballSpeed = 3;
		paddle = {blockPadding, gameHeight - blockPadding - paddleHeight, paddleWidth, paddleHeight};
		normalizeVelocity();
	}

	std::list<Rectangle>::iterator Breakout::getBlockIntersection() {
		if (ballPosition.y - ballSize <= blocksBottom) {
			for (auto iter = blocks.begin(), end = blocks.end(); iter != end; ++iter) {
				if (checkCircleRectangleIntersection(ballPosition, ballSize, *iter)) {
					return iter;
				}
			}
		}

		return blocks.end();
	}

	void Breakout::normalizeVelocity() {
		const double current_speed = ballVelocity.magnitude();
		ballVelocity.x /= current_speed;
		ballVelocity.y /= current_speed;
		ballVelocity.x *= ballSpeed;
		ballVelocity.y *= ballSpeed;
	}

	void Breakout::breakBlock(std::list<Rectangle>::iterator iter) {
		blocks.erase(iter);
		increaseScore(BREAKOUT_BLOCK_SCORE);
	}

	void Breakout::ballLost() {
		INFO("Ball lost");
		ballPosition.x = gameWidth / 2;
		ballPosition.y = gameHeight / 2;
	}

	void Breakout::bounceX() {
		ballVelocity.x *= -1;
	}

	void Breakout::bounceY() {
		ballVelocity.y *= -1;
	}
}
