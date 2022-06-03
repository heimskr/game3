// Contains code from nanogui and from LearnOpenGL (https://github.com/JoeyDeVries/LearnOpenGL)

#include <libnoise/noise.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "MarchingSquares.h"
#include "resources.h"
#include "ui/Canvas.h"
#include "util/Timer.h"
#include "util/Util.h"

namespace Game3 {

	Canvas::Canvas(nanogui::Widget *parent_): GLCanvas(parent_) {
		setBackgroundColor({66, 172, 175, 255});

		constexpr double noise_zoom = 100.;
		constexpr double noise_threshold = -0.15;

		tileset = Texture("resources/tileset2.png");

		uint8_t tiles[HEIGHT][WIDTH];

		constexpr int w = sizeof(tiles[0]) / sizeof(tiles[0][0]);
		constexpr int h = sizeof(tiles) / sizeof(tiles[0]);

		int scale = 16;
		magic = scale / 2;
		tilemap = std::make_shared<Tilemap>(w, h, scale, tileset.width, tileset.height, tileset.id);

		noise::module::Perlin perlin;
		perlin.SetSeed(666);

		for (int i = 0; i < w; ++i)
			for (int j = 0; j < h; ++j) {
				double noise = perlin.GetValue(i / noise_zoom, j / noise_zoom, 0.666);
				uint8_t &tile = tiles[j][i];
				if (noise < noise_threshold) {
					tile = DEEPER_WATER;
				} else if (noise < noise_threshold + 0.1) {
					tile = DEEP_WATER;
				} else if (noise < noise_threshold + 0.2) {
					tile = WATER;
				} else if (noise < noise_threshold + 0.3) {
					tile = SHALLOW_WATER;
				} else if (noise < noise_threshold + 0.4) {
					tile = SAND;
				} else if (noise < noise_threshold + 0.5) {
					tile = LIGHT_GRASS;
				} else {
					constexpr static Tile grasses[] {GRASS_ALT1, GRASS_ALT2, GRASS, GRASS, GRASS, GRASS, GRASS, GRASS, GRASS};
					srand((i << 20) | j);
					tile = grasses[rand() % (sizeof(grasses) / sizeof(grasses[0]))];
				}
			}

		constexpr static int m = 19, n = 15, pad = 2;
		std::vector<unsigned> starts;
		std::vector<unsigned> candidates;
		Timer land_timer("GetLand");
		starts = getLand(tiles, m + pad * 2, n + pad * 2);
		land_timer.stop();
		candidates.reserve(starts.size() / 16);
		Timer candidate_timer("Candidates");
		for (const auto index: starts) {
			const size_t row_start = index / WIDTH + pad, row_end = row_start + m;
			const size_t column_start = index % WIDTH + pad, column_end = column_start + n;
			for (size_t row = row_start; row < row_end; ++row)
				for (size_t column = column_start; column < column_end; ++column)
					if (!isLand(tiles[row][column]))
						goto failed;
			candidates.push_back(index);
			failed:
			continue;
		}
		candidate_timer.stop();
		Timer::summary();

		std::cout << "Found " << candidates.size() << " candidate" << (candidates.size() == 1? "" : "s") << ".\n";
		if (!candidates.empty()) {
			const auto choice = choose(candidates, 666) + pad * (WIDTH + 1);
			std::cout << "Choice: " << choice << '\n';

			size_t row = 0, column = 0;

			for (size_t row = choice / WIDTH; row < choice / WIDTH + m; ++row) {
				tiles[row][choice % WIDTH] = GRAY;
				tiles[row][choice % WIDTH + n - 1] = GRAY;
			}

			for (size_t column = choice % WIDTH; column < choice % WIDTH + n; ++column) {
				tiles[choice / WIDTH][column] = GRAY;
				tiles[choice / WIDTH + m - 1][column] = GRAY;
			}

			auto set = [&](Tile tile) { tiles[row][column] = tile; };

			for (row = choice / WIDTH + 1; row < choice / WIDTH + m - 1; ++row)
				for (column = choice % WIDTH + 1; column < choice % WIDTH + n - 1; ++column)
					set(DIRT);

			row = choice / WIDTH + m / 2;
			for (column = choice % WIDTH - pad; column < choice % WIDTH + n + pad; ++column)
				set(ROAD);
			column = choice % WIDTH + n / 2;
			for (row = choice / WIDTH - pad; row < choice / WIDTH + m + pad; ++row)
				set(ROAD);
			// set(ROAD);
			// --column;
			// set(ROAD);
			// column += n;
			// set(ROAD);
			// ++column;
			// set(ROAD);

			row -= m / 2;
			column -= n - n / 2;

			// tiles[row + m / 2][column] = tiles[row + m / 2][column + n - 1] = ROAD;
			// tiles[row][column + n / 2] = tiles[row + n - 1][column + n / 2] = ROAD;
		}

		for (int r = 0; r < h; ++r)
			for (int c = 0; c < w; ++c)
				(*tilemap)(c, r) = tiles[r][c];

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

		if (!down) {
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

	std::vector<unsigned> Canvas::getLand(uint8_t tiles[WIDTH][HEIGHT], size_t right_pad, size_t bottom_pad) const {
		std::vector<unsigned> land_tiles;
		land_tiles.reserve(WIDTH * HEIGHT);
		for (size_t row = 0; row < HEIGHT - bottom_pad; ++row)
			for (size_t column = 0; column < WIDTH - right_pad; ++column)
				if (isLand(tiles[row][column]))
					land_tiles.push_back(row * WIDTH + column);
		return land_tiles;
	}
}
