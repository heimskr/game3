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
					if (const int area = orient2D(v0, v1, v2); area == 0)
						return;

					// Compute triangle bounding box
					int min_x = std::min({int(v0.x), int(v1.x), int(v2.x)});
					int min_y = std::min({int(v0.y), int(v1.y), int(v2.y)});
					int max_x = std::min({int(v0.x), int(v1.x), int(v2.x)});
					int max_y = std::min({int(v0.y), int(v1.y), int(v2.y)});

					// Rasterize
					jcv_point p;
					for (p.y = jcv_real(min_y); p.y <= jcv_real(max_y); p.y++) {
						for (p.x = jcv_real(min_x); p.x <= jcv_real(max_x); p.x++) {
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
								raw[i + p] = (color >> (8 * p)) & 0xff;
							}
						}
					}

					return raw;
				}
		};
	}

	void voronoiTest() {
		Grid grid;
		grid(0, 0) = 0xffffffff;

		auto raw = grid.makeRaw();

		std::stringstream ss;
		stbi_write_png_to_func(+[](void *context, void *data, int size) {
			std::stringstream &ss = *reinterpret_cast<std::stringstream *>(context);
			ss << std::string_view(reinterpret_cast<const char *>(data), size);
		}, &ss, grid.getWidth(), grid.getHeight(), 4, raw.get(), grid.getWidth() * 4);
		std::cout << ss.str();
	}
}
