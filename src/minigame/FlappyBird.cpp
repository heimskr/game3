#include "minigame/FlappyBird.h"

#ifdef ENABLE_ZIP8
#include "game/ClientGame.h"
#include "graphics/Color.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "packet/SubmitScorePacket.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/Window.h"
#include "util/FS.h"
#include "util/Math.h"

namespace Game3 {
	namespace {
		constexpr Color FLAPPYBIRD_FOREGROUND{"#425229"};
		constexpr Color FLAPPYBIRD_BACKGROUND{"#ffffce"};
		constexpr double FLAPPYBIRD_SCORE_MULTIPLIER = 100;
		constexpr double FLAPPYBIRD_SCORE_POWER = 1.1;

		constexpr std::size_t getScore(uint64_t flags) {
			return static_cast<std::size_t>(std::pow(flags, FLAPPYBIRD_SCORE_POWER) * FLAPPYBIRD_SCORE_MULTIPLIER);
		}
	}

	void FlappyBird::tick(double) {
		if (!cpu) {
			return;
		}

		const bool fire = ui.window.isKeyHeld(GLFW_KEY_UP);

		cpu->setKey(0xe, fire);
		cpu->tick(400);

		if (cpu->getFlagsDirty()) {
			dirty = true;
			const uint64_t flags = cpu->getFlags();
			ui.getGame()->send(make<SubmitScorePacket>(ID(), getScore(flags)));
			cpu->clearFlagsDirty();
		}

		if (dirty && fire) {
			reset();
			dirty = false;
		}
	}

	void FlappyBird::render(const RendererContext &renderers, float x, float y, float width, float height) {
		width = gameWidth * getScale();
		height = gameHeight * getScale();

		Minigame::render(renderers, x, y, width, height);

		if (!cpu || !display) {
			return;
		}

		const double scale = getScale();

		renderers.singleSprite.drawOnScreen(display, RenderOptions{
			.x = x,
			.y = y,
			.sizeX = -1,
			.sizeY = -1,
			.scaleX = scale,
			.scaleY = scale,
			.invertY = false,
		});
	}

	void FlappyBird::reset() {
		cpu = std::make_unique<Zip8>(readFile("chip8/flappybird.ch8"));

		if (display) {
			display->destroy();
		}

		display = std::make_shared<Texture>(Identifier{}, false, GL_NEAREST);
		display->init(64, 32);

		cpu->signalDisplayUpdated.connect([this](std::span<const uint8_t> raw) {
			std::array<uint8_t, 64 * 32 * 3> pixels;
			std::size_t index = 0;

			constexpr static uint8_t red_on = FLAPPYBIRD_FOREGROUND.red * 255;
			constexpr static uint8_t green_on = FLAPPYBIRD_FOREGROUND.green * 255;
			constexpr static uint8_t blue_on = FLAPPYBIRD_FOREGROUND.blue * 255;
			constexpr static uint8_t red_off = FLAPPYBIRD_BACKGROUND.red * 255;
			constexpr static uint8_t green_off = FLAPPYBIRD_BACKGROUND.green * 255;
			constexpr static uint8_t blue_off = FLAPPYBIRD_BACKGROUND.blue * 255;

			for (int row = 0; row < 32; ++row) {
				for (int column = 0; column < 64 / 8; ++column) {
					uint8_t packed = raw[row * 8 + column];
					for (int bit = 0; bit < 8; ++bit) {
						const bool on = packed & 1;
						packed >>= 1;
						pixels[index++] = on? red_on : red_off;
						pixels[index++] = on? green_on : green_off;
						pixels[index++] = on? blue_on : blue_off;
					}
				}
			}

			display->upload(pixels);
		});
	}
}
#endif
