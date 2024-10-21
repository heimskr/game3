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
	}

	Breakout::Breakout():
		ballSize(30),
		blockWidth(50),
		blockHeight(30),
		blockPadding(10),
		paddleWidth(100),
		paddleHeight(30),
		paddleSpeed(8),
		rowCount(6) {}

	void Breakout::tick(UIContext &ui, double delta) {
		accumulatedTime += delta;
		if (accumulatedTime < BREAKOUT_CHECK_TIME) {
			return;
		}

		accumulatedTime = 0;

		ballPosition.x += ballVelocity.x;
		ballPosition.y += ballVelocity.y;

		if (ballPosition.x + ballSize >= gameWidth) {
			ballPosition.x = gameWidth - ballSize;
			bounceX();
		} else if (ballPosition.x <= 0) {
			ballPosition.x = 0;
			bounceX();
		}

		if (ballPosition.y >= gameHeight) {
			ballLost();
		} else if (ballPosition.y <= 0) {
			ballPosition.y = 0;
			bounceY();
		}

		Rectangle ball(ballPosition, ballSize);

		if (paddle.intersection(ball) && paddle.y <= ballPosition.y + ballSize && ballPosition.y + ballSize <= paddle.y + paddle.height) {
			// https://gamedev.stackexchange.com/a/21048
			bounceY();
			ballPosition.y = paddle.y - ballSize - 1;
			const int paddle_center = paddle.x + paddle.width / 2;
			const double pos_x = (ballPosition.x + ballSize / 2 - paddle_center) / (paddle.width / 2.0);
			constexpr double influence = 0.75;
			ballVelocity.x = ballSpeed * pos_x * influence;
			normalizeVelocity();
		}

		const bool left_held = ui.window.isKeyHeld(GLFW_KEY_LEFT);
		const bool right_held = ui.window.isKeyHeld(GLFW_KEY_RIGHT);


		if (left_held) {
			Rectangle new_paddle = paddle;
			new_paddle.x = std::max(0, new_paddle.x - paddleSpeed);
			const int right = ballPosition.x + ballSize;
			if (new_paddle.intersection(ball) && new_paddle.x <= right && right <= new_paddle.x + new_paddle.width) {
				bounceX();
				new_paddle.x = right;
			}
			paddle = new_paddle;
		} else if (right_held) {
			Rectangle new_paddle = paddle;
			new_paddle.x = std::min(gameWidth - paddle.width, new_paddle.x + paddleSpeed);
			const int left = ballPosition.x;
			if (new_paddle.intersection(ball) && new_paddle.x <= left && left <= new_paddle.x + new_paddle.width) {
				bounceX();
				new_paddle.x = left - paddle.width;
			}
			paddle = new_paddle;
		}

		auto iter = getBlockIntersection();
		if (iter != blocks.end()) {
			Rectangle intersection = Rectangle(ballPosition, ballSize).intersection(*iter);

			const auto comparison = intersection.width <=> intersection.height;

			if (comparison == std::strong_ordering::less) {
				bounceX();
			} else if (comparison == std::strong_ordering::greater) {
				bounceY();
			} else {
				bounceX();
				bounceY();
			}

			breakBlock(iter);
		}
	}

	void Breakout::render(UIContext &ui, const RendererContext &renderers) {
		RectangleRenderer &rectangler = renderers.rectangle;

		rectangler.drawOnScreen(BREAKOUT_BACKGROUND, 0, 0, gameWidth, gameHeight);

		for (const Rectangle &block: blocks) {
			rectangler.drawOnScreen(BREAKOUT_FOREGROUND, block);
		}

		rectangler.drawOnScreen(BREAKOUT_FOREGROUND, paddle);
		rectangler.drawOnScreen(BREAKOUT_FOREGROUND, Rectangle(ballPosition, ballSize));
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
		resetBallPosition();
		ballVelocity = {0, 1};
		ballSpeed = 3;
		paddle = {(gameWidth - paddleWidth) / 2, gameHeight - blockPadding - paddleHeight, paddleWidth, paddleHeight};
		normalizeVelocity();
	}

	std::list<Rectangle>::iterator Breakout::getBlockIntersection() {
		if (ballPosition.y <= blocksBottom) {
			Rectangle ball(ballPosition, ballSize);
			for (auto iter = blocks.begin(), end = blocks.end(); iter != end; ++iter) {
				if (ball.intersection(*iter)) {
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
		resetBallPosition();
		ballVelocity = {0, 1};
		normalizeVelocity();
	}

	void Breakout::bounceX() {
		ballVelocity.x *= -1;
	}

	void Breakout::bounceY() {
		ballVelocity.y *= -1;
	}

	void Breakout::resetBallPosition() {
		ballPosition = Vector2d((gameWidth - ballSize) / 2, blocksBottom);
	}
}
