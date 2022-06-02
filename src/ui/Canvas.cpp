// Contains code from nanogui and from LearnOpenGL (https://github.com/JoeyDeVries/LearnOpenGL)

#include <libnoise/noise.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "MarchingSquares.h"
#include "resources.h"
#include "ui/Canvas.h"

namespace Game3 {

	Canvas::Canvas(nanogui::Widget *parent_): GLCanvas(parent_) {
		setBackgroundColor({20, 20, 255, 255});

		constexpr double noise_zoom = 20.;
		constexpr double noise_threshold = -0.15;

		tileset = Texture("resources/tileset2.png");

		int ints[20][20];

		constexpr static int w = sizeof(ints[0]) / sizeof(ints[0][0]);
		constexpr static int h = sizeof(ints) / sizeof(ints[0]);

		int scale = 16;
		magic = scale / 2;
		tilemap = std::make_shared<Tilemap>(w, h, scale, tileset.width, tileset.height, tileset.id);

		static int r = 0;
		static int c = 0;

		noise::module::Perlin perlin;
		perlin.SetSeed(666);

		for (int i = 0; i < w; ++i)
			for (int j = 0; j < h; ++j) {
				double noise = perlin.GetValue(i / noise_zoom, j / noise_zoom, 0.666);
				int &tile = ints[j][i];
				if (noise < noise_threshold) {
					tile = 4;
				} else if (noise < noise_threshold + 0.1) {
					tile = 3;
				} else if (noise < noise_threshold + 0.2) {
					tile = 2;
				} else if (noise < noise_threshold + 0.3) {
					tile = 1;
				} else if (noise < noise_threshold + 0.4) {
					tile = 0;
				} else if (noise < noise_threshold + 0.5) {
					tile = 11;
				} else {
					constexpr static int full[] {40, 41, 12, 12, 12, 12, 12, 12, 12};
					srand((i << 20) | j);
					tile = full[rand() % (sizeof(full) / sizeof(full[0]))];
				}
			}

		for (r = 0; r < h; ++r)
			for (c = 0; c < w; ++c)
				(*tilemap)(c, r) = ints[r][c];

		srand(time(nullptr));
		tilemapRenderer.initialize(tilemap);
	}

	void Canvas::draw(NVGcontext *context_) {
		nanogui::GLCanvas::draw(context_);
		if (context_ != nullptr && context != context_) {
			context = context_;
			font = nvgCreateFont(context, "FreeSans", "resources/FreeSans.ttf");
		}
	}

	void Canvas::drawGL() {
		tilemapRenderer.onBackBufferResized(width(), height());
		tilemapRenderer.render(context, font);
	}

	bool Canvas::scrollEvent(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) {
		if (nanogui::GLCanvas::scrollEvent(p, rel))
			return true;

		if (rel.y() == 1)
			tilemapRenderer.scale *= 1.06f;
		else if (rel.y() == -1)
			tilemapRenderer.scale /= 1.06f;

		return true;
	}

	bool Canvas::mouseDragEvent(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) {
		if (nanogui::GLCanvas::mouseDragEvent(p, rel, button, modifiers))
			return true;

		if (button == 1) {
			center().x() += rel.x() / (magic * tilemapRenderer.scale);
			center().y() += rel.y() / (magic * tilemapRenderer.scale);
			return true;
		}

		return false;
	}

	bool Canvas::mouseButtonEvent(const nanogui::Vector2i &p, int button, bool down, int modifiers) {
		if (nanogui::GLCanvas::mouseButtonEvent(p, button, down, modifiers))
			return true;

		if (down) {
			float fx = p.x();
			float fy = p.y() - HEADER_HEIGHT / 2.f;

			fx -= width() / 2.f - (tilemap->width * tilemap->tileSize / 4.f) * scale() + center().x() * magic * scale();
			fx /= tilemap->tileSize * scale() / 2.f;

			fy -= height() / 2.f - (tilemap->height * tilemap->tileSize / 4.f) * scale() + center().y() * magic * scale();
			fy /= tilemap->tileSize * scale() / 2.f;

			int x = fx;
			int y = fy;

			std::cerr << x << ", " << y << '\n';

			return true;
		}


		return false;
	}
}
