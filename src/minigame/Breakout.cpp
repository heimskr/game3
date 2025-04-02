#include "game/ClientGame.h"
#include "graphics/Color.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "minigame/Breakout.h"
#include "packet/SubmitScorePacket.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/Window.h"
#include "util/Math.h"

namespace Game3 {
	namespace {
		constexpr Color BREAKOUT_FOREGROUND{"#425229"};
		constexpr Color BREAKOUT_BACKGROUND{"#ffffce"};
		constexpr double BREAKOUT_CHECK_TIME = 0.005;
		constexpr std::size_t BREAKOUT_BLOCK_SCORE = 25;
		constexpr int BREAKOUT_INITIAL_LIVES = 3;
	}

	Breakout::Breakout(UIContext &ui, float scale):
		Minigame(ui, scale),
		ballSize(30),
		blockWidth(50),
		blockHeight(30),
		blockPadding(10),
		paddleWidth(100),
		paddleHeight(30),
		paddleSpeed(8),
		rowCount(6) {}

	void Breakout::tick(double delta) {
		if (submitQueued) {
			submitScore();
			submitQueued = false;
		}

		accumulatedTime += delta;
		if (accumulatedTime < BREAKOUT_CHECK_TIME) {
			return;
		}

		accumulatedTime = 0;

		if (isGameOver) {
			return;
		}

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

		if (ui.window.isKeyHeld(GLFW_KEY_LEFT)) {
			Rectangle new_paddle = paddle;
			new_paddle.x = std::max(0, new_paddle.x - paddleSpeed);
			const int right = ballPosition.x + ballSize;
			if (new_paddle.intersection(ball) && new_paddle.x <= right && right <= new_paddle.x + new_paddle.width) {
				bounceX();
				new_paddle.x = right;
			}
			paddle = new_paddle;
		} else if (ui.window.isKeyHeld(GLFW_KEY_RIGHT)) {
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

	void Breakout::render(const RendererContext &renderers, float x, float y, float width, float height) {
		width = gameWidth;
		height = gameHeight;

		Widget::render(renderers, x, y, width, height);

		TextRenderer &texter = renderers.text;
		RectangleRenderer &rectangler = renderers.rectangle;
		rectangler.drawOnScreen(BREAKOUT_BACKGROUND, x, y, gameWidth, gameHeight);

		if (isGameOver) {
			texter("GAME OVER", TextRenderOptions{
				.x = x + static_cast<double>(gameWidth / 2),
				.y = y + static_cast<double>(gameHeight / 2),
				.color = BREAKOUT_FOREGROUND,
				.align = TextAlign::Center,
			});
			return;
		}

		for (const Rectangle &block: blocks) {
			rectangler.drawOnScreen(BREAKOUT_FOREGROUND, Rectangle(x, y) + block);
		}

		rectangler.drawOnScreen(BREAKOUT_FOREGROUND, Rectangle(x, y) + paddle);
		rectangler.drawOnScreen(BREAKOUT_FOREGROUND, Rectangle(x, y) + Rectangle(ballPosition, ballSize));

		texter(std::format("{}", score), TextRenderOptions{
			.x = x + static_cast<double>(blockPadding),
			.y = y + static_cast<double>(gameHeight - blockPadding),
			.scaleX = 0.5,
			.scaleY = 0.5,
			.color = BREAKOUT_FOREGROUND,
		});

		const int shrunk = ballSize * 0.5;
		x += gameWidth - shrunk - blockPadding;

		for (int i = 0; i < lives; ++i) {
			rectangler.drawOnScreen(BREAKOUT_FOREGROUND, x, y + gameHeight - blockPadding - shrunk, shrunk, shrunk);
			x -= shrunk + blockPadding;
		}
	}

	void Breakout::reset() {
		lives = BREAKOUT_INITIAL_LIVES;
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
		paddle = {(gameWidth - paddleWidth) / 2, gameHeight - blockPadding - paddleHeight - 40, paddleWidth, paddleHeight};
		normalizeVelocity();
	}

	void Breakout::onClose() {
		submitScore();
	}

	void Breakout::submitScore() {
		ui.getGame()->send(make<SubmitScorePacket>(ID(), score));
		score = 0;
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
		if (--lives == 0) {
			gameOver();
		} else {
			resetBallPosition();
			ballVelocity = {0, 1};
			normalizeVelocity();
		}
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

	void Breakout::gameOver() {
		isGameOver = true;
		submitQueued = true;
	}
}
