#pragma once

#include <random>

#include <gdkmm/rectangle.h>
#include <Eigen/Eigen>

#include "Position.h"
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
			Eigen::Vector2f center {0.f, 0.f};
			float scale = DEFAULT_SCALE;
			SpriteRenderer spriteRenderer {*this};
			RectangleRenderer rectangleRenderer;
			float magic = 8.f;
			int autofocusCounter = 0;
			Gdk::Rectangle realmBounds;

			Canvas(MainWindow &);

			void drawGL();
			// bool mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers) override;
			int width() const;
			int height() const;

			/** Recalculates the visible realm bounds (see Game::getVisibleRealmBounds). */
			void update();
			bool inBounds(const Position &) const;
	};
}
