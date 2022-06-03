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

		uint8_t tiles1[HEIGHT][WIDTH];
		uint8_t tiles2[HEIGHT][WIDTH];

		std::memset(tiles2, 0, sizeof(tiles2));

		int scale = 16;
		magic = scale / 2;
		tilemap1 = std::make_shared<Tilemap>(WIDTH, HEIGHT, scale, tileset.width, tileset.height, tileset.id);
		tilemap2 = std::make_shared<Tilemap>(WIDTH, HEIGHT, scale, tileset.width, tileset.height, tileset.id);

		noise::module::Perlin perlin;
		perlin.SetSeed(666);

		for (int i = 0; i < WIDTH; ++i)
			for (int j = 0; j < HEIGHT; ++j) {
				double noise = perlin.GetValue(i / noise_zoom, j / noise_zoom, 0.666);
				uint8_t &tile = tiles1[j][i];
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

		constexpr static int m = 15, n = 21, pad = 2;
		std::vector<unsigned> starts;
		std::vector<unsigned> candidates;
		Timer land_timer("GetLand");
		starts = getLand(tiles1, m + pad * 2, n + pad * 2);
		land_timer.stop();
		candidates.reserve(starts.size() / 16);
		Timer candidate_timer("Candidates");
		for (const auto index: starts) {
			const size_t row_start = index / WIDTH + pad, row_end = row_start + m;
			const size_t column_start = index % WIDTH + pad, column_end = column_start + n;
			for (size_t row = row_start; row < row_end; ++row)
				for (size_t column = column_start; column < column_end; ++column)
					if (!isLand(tiles1[row][column]))
						goto failed;
			candidates.push_back(index);
			failed:
			continue;
		}
		candidate_timer.stop();
		Timer::summary();

		std::cout << "Found " << candidates.size() << " candidate" << (candidates.size() == 1? "" : "s") << ".\n";
		if (!candidates.empty())
			createTown(tiles1, tiles2, choose(candidates, 666) + pad * (WIDTH + 1), n, m, pad);

		for (int r = 0; r < HEIGHT; ++r)
			for (int c = 0; c < WIDTH; ++c) {
				(*tilemap1)(c, r) = tiles1[r][c];
				(*tilemap2)(c, r) = tiles2[r][c];
			}

		srand(time(nullptr));
		tilemapRenderer1.initialize(tilemap1);
		tilemapRenderer2.initialize(tilemap2);
	}

	void Canvas::draw(NVGcontext *context_) {
		nanogui::GLCanvas::draw(context_);
		if (context_ != nullptr && context != context_) {
			context = context_;
			font = nvgCreateFont(context, "FreeSans", "resources/FreeSans.ttf");
		}
	}

	void Canvas::drawGL() {
		tilemapRenderer1.onBackBufferResized(width(), height());
		tilemapRenderer2.onBackBufferResized(width(), height());
		tilemapRenderer1.render(context, font);
		tilemapRenderer2.render(context, font);
	}

	bool Canvas::scrollEvent(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) {
		if (nanogui::GLCanvas::scrollEvent(p, rel))
			return true;

		if (rel.y() == 1)
			scale(scale() * 1.06f);
		else if (rel.y() == -1)
			scale(scale() / 1.06f);

		return true;
	}

	bool Canvas::mouseDragEvent(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) {
		if (nanogui::GLCanvas::mouseDragEvent(p, rel, button, modifiers))
			return true;

		if (button == 1) {
			auto vec = center();
			vec.x() += rel.x() / (magic * scale());
			vec.y() += rel.y() / (magic * scale());
			center(vec);
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

			fx -= width() / 2.f - (WIDTH * tilemap1->tileSize / 4.f) * scale() + center().x() * magic * scale();
			fx /= tilemap1->tileSize * scale() / 2.f;

			fy -= height() / 2.f - (HEIGHT * tilemap1->tileSize / 4.f) * scale() + center().y() * magic * scale();
			fy /= tilemap1->tileSize * scale() / 2.f;

			int x = fx;
			int y = fy;

			std::cerr << x << ", " << y << '\n';

			return true;
		}

		return false;
	}

	std::vector<unsigned> Canvas::getLand(uint8_t tiles[HEIGHT][WIDTH], size_t right_pad, size_t bottom_pad) const {
		std::vector<unsigned> land_tiles;
		land_tiles.reserve(WIDTH * HEIGHT);
		for (size_t row = 0; row < HEIGHT - bottom_pad; ++row)
			for (size_t column = 0; column < WIDTH - right_pad; ++column)
				if (isLand(tiles[row][column]))
					land_tiles.push_back(row * WIDTH + column);
		return land_tiles;
	}

	void Canvas::createTown(uint8_t layer1[HEIGHT][WIDTH], uint8_t layer2[HEIGHT][WIDTH], size_t index, size_t width, size_t height, size_t pad) const {
		size_t row = 0, column = 0;

		auto set1 = [&](Tile tile) { layer1[row][column] = tile; };
		auto set2 = [&](Tile tile) { layer2[row][column] = tile; };

		for (size_t row = index / WIDTH; row < index / WIDTH + height; ++row) {
			layer2[row][index % WIDTH] = TOWER_NS;
			layer2[row][index % WIDTH + width - 1] = TOWER_NS;
		}

		for (size_t column = index % WIDTH; column < index % WIDTH + width; ++column) {
			layer2[index / WIDTH][column] = TOWER_WE;
			layer2[index / WIDTH + height - 1][column] = TOWER_WE;
		}

		layer2[index / WIDTH][index % WIDTH] = TOWER_NW;

		for (row = index / WIDTH; row < index / WIDTH + height; ++row)
			for (column = index % WIDTH; column < index % WIDTH + width; ++column)
				set1(DIRT);

		row = index / WIDTH + height / 2;
		for (column = index % WIDTH - pad; column < index % WIDTH + width + pad; ++column)
			set1(ROAD);
		column = index % WIDTH + width / 2;
		for (row = index / WIDTH - pad; row < index / WIDTH + height + pad; ++row)
			set1(ROAD);
	}
}
