#pragma once

#include <random>

#include <nanogui/opengl.h>
#include <nanogui/common.h>

#include "Texture.h"
#include "Types.h"
#include "ui/RectangleRenderer.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	class Game;
	class MainWindow;

	class Canvas {
		public:
			constexpr static float DEFAULT_SCALE = 2.f;
			constexpr static int AUTOFOCUS_DELAY = 1;

			MainWindow &window;
			std::shared_ptr<Game> game;
			nanogui::Vector2f center {0.f, 0.f};
			float scale = DEFAULT_SCALE;
			SpriteRenderer spriteRenderer {*this};
			RectangleRenderer rectangleRenderer {*this};
			float magic = 8.f;
			int autofocusCounter = 0;

			Canvas(MainWindow &);

			void drawGL();
			// bool mouseButtonEvent(const nanogui::Vector2i &p, int button, bool down, int modifiers) override;
			int width();
			int height();
	};
}
