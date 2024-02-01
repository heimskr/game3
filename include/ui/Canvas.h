#pragma once

#include <random>

#include <gdkmm/rectangle.h>
#include "lib/Eigen.h"

#include "types/Position.h"
#include "graphics/Texture.h"
#include "types/Types.h"
#include "graphics/GL.h"
#include "graphics/BatchSpriteRenderer.h"
#include "graphics/CircleRenderer.h"
#include "graphics/Multiplier.h"
#include "graphics/Overlayer.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/TextRenderer.h"

namespace Game3 {
	class ClientGame;
	class MainWindow;
	class Realm;
	struct RendererContext;

	class Canvas {
		public:
			constexpr static double DEFAULT_SCALE = 8.;
			constexpr static int AUTOFOCUS_DELAY = 1;

			MainWindow &window;
			std::shared_ptr<ClientGame> game;
			std::pair<double, double> center{0., 0.};
			double scale = DEFAULT_SCALE;
			BatchSpriteRenderer  batchSpriteRenderer{*this};
			SingleSpriteRenderer singleSpriteRenderer{*this};
			TextRenderer textRenderer{*this};
			RectangleRenderer rectangleRenderer{*this};
			CircleRenderer circleRenderer{*this};
			GL::Texture mainTexture;
			GL::Texture staticLightingTexture;
			GL::Texture dynamicLightingTexture;
			GL::FBO fbo;
			Multiplier multiplier;
			Overlayer overlayer;
			float magic = 8.f;
			int autofocusCounter = 0;
			Gdk::Rectangle realmBounds;
			const Realm *lastRealm = nullptr;
			double sizeDivisor = 1.0;

			Canvas(MainWindow &);

			void drawGL();
			int getWidth() const;
			int getHeight() const;
			int getFactor() const;

			bool inBounds(const Position &) const;

			RendererContext getRendererContext();

		private:
			GL::Texture scratchTexture;
	};
}
