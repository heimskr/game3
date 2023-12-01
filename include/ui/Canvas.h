#pragma once

#include <random>

#include <gdkmm/rectangle.h>
#include "lib/Eigen.h"

#include "types/Position.h"
#include "graphics/Texture.h"
#include "types/Types.h"
#include "graphics/GL.h"
#include "graphics/Multiplier.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/TextRenderer.h"

namespace Game3 {
	class ClientGame;
	class MainWindow;
	class Realm;

	class Canvas {
		public:
			constexpr static double DEFAULT_SCALE = 8.;
			constexpr static int AUTOFOCUS_DELAY = 1;

			MainWindow &window;
			std::shared_ptr<ClientGame> game;
			std::pair<double, double> center{0., 0.};
			double scale = DEFAULT_SCALE;
			std::unique_ptr<SpriteRenderer> spriteRenderer;
			TextRenderer textRenderer{*this};
			RectangleRenderer rectangleRenderer{*this};
			GL::Texture textureA;
			GL::Texture textureB;
			GL::FBO fbo;
			Multiplier multiplier;
			float magic = 8.f;
			int autofocusCounter = 0;
			Gdk::Rectangle realmBounds;
			const Realm *lastRealm = nullptr;

			Canvas(MainWindow &);

			void drawGL();
			// bool mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers) override;
			int width() const;
			int height() const;

			bool inBounds(const Position &) const;
	};
}
