#include "game/ClientGame.h"
#include "graphics/Color.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "minigame/MathGame.h"
#include "packet/SubmitScorePacket.h"
#include "threading/ThreadContext.h"
#include "ui/gl/widget/ProgressBar.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/Window.h"
#include "util/Util.h"

namespace Game3 {
	namespace {
		constexpr Color MATH_GAME_FOREGROUND{"#425229"};
		constexpr Color MATH_GAME_BACKGROUND{"#ffffce"};
	}

	void MathGame::init() {
		equationColor = MATH_GAME_FOREGROUND;

		input = std::make_shared<TextInput>(ui, selfScale, MATH_GAME_FOREGROUND, MATH_GAME_BACKGROUND, MATH_GAME_FOREGROUND, MATH_GAME_FOREGROUND);
		input->onSubmit.connect([this](TextInput &, const UString &text) {
			if (!equation) {
				return;
			}

			int64_t parsed{};

			try {
				parsed = parseNumber<int64_t>(text.raw());
			} catch (const std::invalid_argument &) {
				invalidInput();
				return;
			}

			if (parsed == equation->answer) {
				increaseScore(equation->getRemainingPoints());
				validInput();
				equation = Equation::generate();
			} else {
				invalidInput();
			}
		});
		input->insertAtEnd(shared_from_this());

		bar = std::make_shared<ProgressBar>(ui, selfScale, MATH_GAME_FOREGROUND, MATH_GAME_BACKGROUND.darken(), MATH_GAME_BACKGROUND.darken(3));
		bar->insertAtEnd(shared_from_this());

		setSize(gameWidth, gameHeight);
	}

	void MathGame::tick(double delta) {
		if (!equation) {
			equation = Equation::generate();
			return;
		}

		equation->secondsLeft -= delta;
		if (equation->secondsLeft <= 0) {
			submitScore();
			reset();
			return;
		}

		bar->setProgress(equation->secondsLeft / equation->duration);
	}

	void MathGame::render(const RendererContext &renderers, float x, float y, float width, float height) {
		width = gameWidth;
		height = gameHeight;

		Minigame::render(renderers, x, y, width, height);

		if (!equation) {
			equation = Equation::generate();
			validInput();
		}

		renderers.rectangle.drawOnScreen(MATH_GAME_BACKGROUND, x, y, gameWidth, gameHeight);

		TextRenderer &texter = renderers.text;

		y += 5;
		float text_height{};
		texter.drawOnScreen(equation->text, TextRenderOptions{
			.x = x + gameWidth / 2.0,
			.y = y,
			.color = equationColor,
			.align = TextAlign::Center,
			.alignTop = true,
			.shadow{},
			.heightOut = &text_height,
		});

		texter.drawOnScreen(std::to_string(score), TextRenderOptions{
			.x = x + 5,
			.y = y + gameHeight - 10,
			.scaleX = 0.5,
			.scaleY = 0.5,
			.color = MATH_GAME_FOREGROUND,
			.alignTop = false,
			.shadow{},
		});

		y += text_height + 5;

		const auto input_width = input->getFixedWidth();
		const auto input_height = input->getFixedHeight();
		input->render(renderers, x + (gameWidth - input_width) / 2.0, y, input_width, input_height);

		y += input_height + 10;

		const auto bar_width = bar->getFixedWidth();
		bar->render(renderers, x + (gameWidth - bar_width) / 2.0, y, bar_width, bar->getFixedHeight());
	}

	void MathGame::setSize(int width, int height) {
		Minigame::setSize(width, height);
		const float scale = getScale();
		input->setFixedSize(width - 2 * scale, 10 * scale);
		bar->setFixedSize(width - 2 * scale, 8 * scale);
	}

	void MathGame::reset() {
		equation = Equation::generate();
		score = 0;
		validInput();
	}

	void MathGame::onFocus() {
		ui.focusWidget(input);
	}

	void MathGame::onClose() {
		submitScore();
	}

	void MathGame::submitScore() {
		ui.getGame()->send(make<SubmitScorePacket>(ID(), score));
		score = 0;
	}

	MathGame::Equation::Equation(UString text, int64_t answer, std::size_t points, double duration):
		text(std::move(text)),
		answer(answer),
		points(points),
		duration(duration),
		secondsLeft(duration),
		generationTime(std::chrono::system_clock::now()) {}

	std::size_t MathGame::Equation::getRemainingPoints() const {
		return (secondsLeft / duration) * points;
	}

	static std::size_t getMultiplicationScore(int64_t left, int64_t right) {
		auto ease = [](auto num) { return num % 10 == 0? num / 10 : num; };
		const double base = ease(left) * ease(right);
		return static_cast<std::size_t>(std::max(50.0, std::pow(base, 0.8)));
	}

	auto MathGame::Equation::generate() -> Equation {
		constexpr int64_t min = 0, max = 99;
		const int64_t left  = threadContext.random(min, max);
		const int64_t right = threadContext.random(min, max);
		switch (threadContext.random(0, 2)) {
			case 0:  return {std::format("{} + {} =", left, right), left + right, 40, 10.0};
			case 1:  return {std::format("{} - {} =", left, right), left - right, 50, 20.0};
			case 2:  return {std::format("{} * {} =", left, right), left * right, getMultiplicationScore(left, right), 60.0};
			default: return {"0 =", 0, 0, 60.0};
		}
	}

	void MathGame::invalidInput() {
		equationColor = Color{"#ff0000"};
		input->clear();
	}

	void MathGame::validInput() {
		equationColor = MATH_GAME_FOREGROUND;
		input->clear();
	}
}
