#include "game/ClientGame.h"
#include "graphics/Color.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "minigame/MathGame.h"
#include "packet/SubmitScorePacket.h"
#include "threading/ThreadContext.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/Window.h"

namespace Game3 {
	namespace {
		constexpr Color MATH_FOREGROUND{"#425229"};
		constexpr Color MATH_BACKGROUND{"#ffffce"};
	}

	void MathGame::init() {
		input = std::make_shared<TextInput>(ui, scale);
		input->setFixedSize(gameWidth, 10 * scale);
		input->insertAtEnd(shared_from_this());
	}

	void MathGame::tick(double) {

	}

	void MathGame::render(const RendererContext &renderers, float x, float y, float width, float height) {
		width = gameWidth;
		height = gameHeight;

		Minigame::render(renderers, x, y, width, height);

		if (!equation) {
			equation = Equation::generate();
		}

		renderers.rectangle.drawOnScreen(MATH_BACKGROUND, x, y, gameWidth, gameHeight);

		TextRenderer &texter = renderers.text;

		y += 5;
		float text_height{};
		texter.drawOnScreen(equation->text, TextRenderOptions{
			.x = x + gameWidth / 2.0,
			.y = y,
			.color = MATH_FOREGROUND,
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
			.color = MATH_FOREGROUND,
			.alignTop = false,
			.shadow{},
		});

		y += text_height + 5;

		input->render(renderers, x, y, gameWidth, 10 * scale);
	}

	void MathGame::setSize(int width, int height) {
		Minigame::setSize(width, height);
		input->setFixedSize(width, 10 * scale);
	}

	void MathGame::reset() {
		equation = Equation::generate();
	}

	auto MathGame::Equation::generate() -> Equation {
		constexpr int64_t min = 0, max = 99;
		const int64_t left  = threadContext.random(min, max);
		const int64_t right = threadContext.random(min, max);
		switch (threadContext.random(0, 2)) {
			case 0:  return {std::format("{} + {} =", left, right), left + right, static_cast<std::size_t>(std::max<int64_t>(10, (left + right)) * 10)};
			case 1:  return {std::format("{} - {} =", left, right), left - right, static_cast<std::size_t>(std::max<int64_t>(10, (left + right)) * 10)};
			case 2:  return {std::format("{} * {} =", left, right), left * right, static_cast<std::size_t>(left * right)};
			default: return {"0 =", 0, 0};
		}
	}
}
