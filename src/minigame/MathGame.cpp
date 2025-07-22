#include "game/ClientGame.h"
#include "graphics/Color.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "minigame/MathGame.h"
#include "packet/SubmitScorePacket.h"
#include "threading/ThreadContext.h"
#include "ui/widget/ProgressBar.h"
#include "ui/widget/TextInput.h"
#include "ui/Constants.h"
#include "ui/UIContext.h"
#include "ui/Window.h"
#include "util/Util.h"

namespace Game3 {
	namespace {
		constexpr Color MATH_GAME_FOREGROUND{"#425229"};
		constexpr Color MATH_GAME_BACKGROUND{"#ffffce"};
	}

	void MathGame::init() {
		equationColor = MATH_GAME_FOREGROUND;

		input = make<TextInput>(ui, selfScale, MATH_GAME_FOREGROUND, MATH_GAME_BACKGROUND, MATH_GAME_FOREGROUND, MATH_GAME_FOREGROUND);
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
		input->insertAtEnd(getSelf());

		bar = make<ProgressBar>(ui, selfScale, MATH_GAME_FOREGROUND, MATH_GAME_BACKGROUND.darken(), MATH_GAME_BACKGROUND.darken(3));
		bar->insertAtEnd(getSelf());

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

		renderers.rectangle.drawOnScreen(MATH_GAME_BACKGROUND, x, y, width, height);

		TextRenderer &texter = renderers.text;

		y += 5 * ui.scale;
		float text_height{};
		texter.drawOnScreen(equation->text, TextRenderOptions{
			.x = x + width / 2.0,
			.y = y,
			.scaleX = ui.scale / 8,
			.scaleY = ui.scale / 8,
			.color = equationColor,
			.align = TextAlign::Center,
			.alignTop = true,
			.shadow{},
			.heightOut = &text_height,
		});

		texter.drawOnScreen(std::to_string(score), TextRenderOptions{
			.x = x + 5 * ui.scale,
			.y = y + height - 10 * ui.scale,
			.scaleX = 0.5 * ui.scale / 8,
			.scaleY = 0.5 * ui.scale / 8,
			.color = MATH_GAME_FOREGROUND,
			.alignTop = false,
			.shadow{},
		});

		y += text_height + 5 + ui.scale;

		const auto input_width = input->getFixedWidth() * ui.scale;
		const auto input_height = input->getFixedHeight() * ui.scale;
		input->render(renderers, x + (width - input_width) / 2.0, y, input_width, input_height);

		y += input_height + 10 * ui.scale;

		const auto bar_width = bar->getFixedWidth() * ui.scale;
		const auto bar_height = bar->getFixedHeight() * ui.scale;
		bar->render(renderers, x + (width - bar_width) / 2.0, y, bar_width, bar_height);
	}

	void MathGame::setSize(int width, int height) {
		Minigame::setSize(width, height);
		input->setFixedSize(width / ui.scale - 2, 10);
		bar->setFixedSize(width / ui.scale - 2, 8);
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
