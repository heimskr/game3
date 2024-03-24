#include "Log.h"
#include "util/Math.h"

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#define JC_VORONOI_IMPLEMENTATION
#include "jc_voronoi.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <utility>

namespace Game3 {
	namespace {
		constexpr int64_t CHUNK_SIZE = 64;

		/** Given chunk_size = 32: [0, 32) -> 0, [32, 64) -> 1, [-32, 0) -> -1 */
		template <typename O = int32_t, typename I>
		O divide(I value, I chunk_size = static_cast<I>(CHUNK_SIZE)) {
			if (0 <= value)
				return static_cast<O>(value / chunk_size);
			return static_cast<O>(-updiv(-value, chunk_size));
		}

		template <typename O = int32_t, typename I>
		O remainder(I value, I chunk_size = static_cast<I>(CHUNK_SIZE)) {
			if (0 <= value)
				return static_cast<O>(value % chunk_size);
			return static_cast<O>((chunk_size - (-value % chunk_size)) % chunk_size);
		}

		class Grid {
			public:
				using Color = uint32_t;

			private:
				std::map<std::pair<int, int>, std::array<Color, CHUNK_SIZE * CHUNK_SIZE>> map;

			public:
				std::optional<int> minX;
				std::optional<int> minY;
				std::optional<int> maxX;
				std::optional<int> maxY;

				Color & operator()(int x, int y) {
					const std::pair pair{divide(x), divide(y)};

					if (!minX || pair.first < *minX)
						minX = pair.first;

					if (!minY || pair.second < *minY)
						minY = pair.second;

					if (!maxX || pair.first > *maxX)
						maxX = pair.first;

					if (!maxY || pair.second > *maxY)
						maxY = pair.second;

					auto iter = map.find(pair);
					if (iter != map.end())
						return iter->second[remainder(y) * CHUNK_SIZE + remainder(x)];
					return map.try_emplace(pair).first->second[remainder(y) * CHUNK_SIZE + remainder(x)];;
				}

				Color operator()(int x, int y) const {
					const std::pair pair{divide(x), divide(y)};
					auto iter = map.find(pair);
					if (iter == map.end())
						return 0;
					return iter->second[remainder(y) * CHUNK_SIZE + remainder(x)];
				}

				static inline int orient2D(const jcv_point &a, const jcv_point &b, const jcv_point &c) {
					return (int(b.x) - int(a.x)) * (int(c.y) - int(a.y)) - (int(b.y) - int(a.y)) * (int(c.x) - int(a.x));
				}

				void drawTriangle(const jcv_point &v0, const jcv_point &v1, const jcv_point &v2, Color color) {
					if (const int area = orient2D(v0, v1, v2); area == 0) {
						return;
					}

					// Compute triangle bounding box
					jcv_real min_x = std::min({v0.x, v1.x, v2.x});
					jcv_real min_y = std::min({v0.y, v1.y, v2.y});
					jcv_real max_x = std::max({v0.x, v1.x, v2.x});
					jcv_real max_y = std::max({v0.y, v1.y, v2.y});

					// Rasterize
					jcv_point p;
					for (p.y = jcv_real(min_y); p.y <= jcv_real(max_y); p.y += 1.0) {
						for (p.x = jcv_real(min_x); p.x <= jcv_real(max_x); p.x += 1.0) {
							// Determine barycentric coordinates
							const int w0 = orient2D(v1, v2, p);
							const int w1 = orient2D(v2, v0, p);
							const int w2 = orient2D(v0, v1, p);

							// If p is on or inside all edges, render pixel.
							if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
								(*this)(p.x, p.y) = color;
							}
						}
					}
				}

				inline auto getWidth() const  { return (*maxX - *minX + 1) * CHUNK_SIZE; }
				inline auto getHeight() const { return (*maxY - *minY + 1) * CHUNK_SIZE; }

				std::unique_ptr<uint8_t[]> makeRaw() const {
					if (!minX || !minY || !maxX || !maxY)
						return nullptr;

					const int width  = (*maxX - *minX + 1) * CHUNK_SIZE;
					const int height = (*maxY - *minY + 1) * CHUNK_SIZE;

					std::unique_ptr<uint8_t[]> raw = std::make_unique<uint8_t[]>(width * height * 4);

					for (auto y = *minY * CHUNK_SIZE; y < (*maxY + 1) * CHUNK_SIZE; ++y) {
						for (auto x = *minX * CHUNK_SIZE; x < (*maxX + 1) * CHUNK_SIZE; ++x) {
							const auto i = 4 * (y * width + x);
							Color color = (*this)(x, y);

							for (int p = 0; p < 4; ++p) {
								raw[i + p] = (color >> (8 * (3 - p))) & 0xff;
							}
						}
					}

					return raw;
				}
		};

		inline jcv_point remap(const jcv_point &pt, const jcv_point &min, const jcv_point &max, const jcv_point &scale) {
			return {
				(pt.x - min.x) / (max.x - min.x) * scale.x,
				(pt.y - min.y) / (max.y - min.y) * scale.y,
			};
		}
	}

	void voronoiTest() {
		Grid grid;

		std::default_random_engine rng(64);

		int count = 16;
		auto points = std::make_unique<jcv_point[]>(count);

		std::uniform_real_distribution point_distribution(0.f, 64.f);
		std::uniform_int_distribution  color_distribution(0, 19);

		std::vector<uint32_t> colors{
			0x0a4f75ff,
			0xce7d3eff,
			0xffe13eff,
			0xe2a795ff,
			0xc9eb4aff,
			0x045f5aff,
			0x6feb4cff,
			0xa516bcff,
			0x300e49ff,
			0x59fae1ff,
			0xb74917ff,
			0x060ea1ff,
			0x039ae9ff,
			0xa67fbeff,
			0xfda380ff,
			0xc9bf9bff,
			0x8e66d7ff,
			0x0cd0d8ff,
			0x2016b8ff,
			0xb49904ff,
		};

		for (int i = 0; i < count; ++i) {
			points[i].x = point_distribution(rng);
			points[i].y = point_distribution(rng);
		}

		const int width(CHUNK_SIZE);
		const int height(CHUNK_SIZE);

		jcv_diagram diagram{};
		jcv_point dimensions{jcv_real(width), jcv_real(height)};
		jcv_rect rect{{0.f, 0.f}, {64.f, 64.f}};

		jcv_diagram_generate(count, points.get(), &rect, nullptr, &diagram);

		const jcv_site *sites = jcv_diagram_get_sites(&diagram);
		for (int i = 0; i < diagram.numsites; ++i) {
			const jcv_site &site = sites[i];
			jcv_point s = remap(site.p, diagram.min, diagram.max, dimensions);
			const auto index = color_distribution(rng);
			const auto color = colors.at(index);

			const jcv_graphedge *e = site.edges;
			while (e) {
				jcv_point p0 = remap(e->pos[0], diagram.min, diagram.max, dimensions);
				jcv_point p1 = remap(e->pos[1], diagram.min, diagram.max, dimensions);
				grid.drawTriangle(s, p0, p1, color);
				e = e->next;
			}
		}

		auto raw = grid.makeRaw();

		std::stringstream ss;
		stbi_write_png_to_func(+[](void *context, void *data, int size) {
			std::stringstream &ss = *reinterpret_cast<std::stringstream *>(context);
			ss << std::string_view(reinterpret_cast<const char *>(data), size);
		}, &ss, grid.getWidth(), grid.getHeight(), 4, raw.get(), grid.getWidth() * 4);
		std::cout << ss.str();
	}
}
