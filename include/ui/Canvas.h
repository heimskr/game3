#pragma once

#include <random>

#include <gdkmm/rectangle.h>

#include "lib/Eigen.h"

#include "resources.h"
#include "Position.h"
#include "Texture.h"
#include "Types.h"
#include "ui/RectangleRenderer.h"
#include "ui/Multiplier.h"
#include "ui/SpriteRenderer.h"
#include "ui/TextRenderer.h"
#include "util/GL.h"

#include <nanogui/nanogui.h>

namespace Game3 {
	class Application;
	class ClientGame;
	class Realm;

	class Canvas: public nanogui::Canvas {
		public:
			constexpr static float DEFAULT_SCALE = 2.f;
			constexpr static int AUTOFOCUS_DELAY = 1;

			Application &app;
			std::shared_ptr<ClientGame> game;
			Eigen::Vector2f center {0.f, 0.f};
			float scale = DEFAULT_SCALE;
			SpriteRenderer spriteRenderer {*this};
			TextRenderer textRenderer {*this};
			RectangleRenderer rectangleRenderer;
			GL::Texture textureA;
			GL::Texture textureB;
			GL::FBO fbo;
			Multiplier multiplier;
			float magic = 8.f;
			int autofocusCounter = 0;
			Gdk::Rectangle realmBounds;
			const Realm *lastRealm = nullptr;

			Canvas(Application &);

			void drawGL();
			// bool mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers) override;
			// int width() const;
			// int height() const;

			bool inBounds(const Position &) const;
	};
}
