#pragma once

#include "container/RectangularVector.h"
#include "jc_voronoi.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <memory>
#include <random>

namespace Game3 {
	template <typename T, typename R>
	void voronoize(RectangularVector<T> &out, int count, R &rng, const std::function<T()> &generator) {
		auto remap = [](const jcv_point &pt, const jcv_point &min, const jcv_point &max, const jcv_point &scale) -> jcv_point {
			return {
				(pt.x - min.x) / (max.x - min.x) * scale.x,
				(pt.y - min.y) / (max.y - min.y) * scale.y,
			};
		};

		auto orient_2d = [](const jcv_point &a, const jcv_point &b, const jcv_point &c) {
			return (int(b.x) - int(a.x)) * (int(c.y) - int(a.y)) - (int(b.y) - int(a.y)) * (int(c.x) - int(a.x));
		};

		auto draw_triangle = [&](const jcv_point &v0, const jcv_point &v1, const jcv_point &v2, T color) -> bool {
			if (const int area = orient_2d(v0, v1, v2); area == 0) {
				return false;
			}

			// Compute triangle bounding box
			jcv_real min_x = std::max(jcv_real(0), std::min({v0.x, v1.x, v2.x}));
			jcv_real min_y = std::max(jcv_real(0), std::min({v0.y, v1.y, v2.y}));
			jcv_real max_x = std::min(jcv_real(out.width),  std::max({v0.x, v1.x, v2.x}));
			jcv_real max_y = std::min(jcv_real(out.height), std::max({v0.y, v1.y, v2.y}));

			// Rasterize
			jcv_point p;
			for (p.y = min_y; p.y <= max_y; p.y += 1.f) {
				for (p.x = min_x; p.x <= max_x; p.x += 1.f) {
					// Determine barycentric coordinates
					const int w0 = orient_2d(v1, v2, p);
					const int w1 = orient_2d(v2, v0, p);
					const int w2 = orient_2d(v0, v1, p);

					// If p is on or inside all edges, render pixel.
					if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
						out[p.x, p.y] = color;
					}
				}
			}

			return true;
		};

		std::uniform_real_distribution x_distribution(0.f, float(out.width));
		std::uniform_real_distribution y_distribution(0.f, float(out.height));

		auto points = std::make_unique<jcv_point[]>(count);
		for (int i = 0; i < count; ++i) {
			points[i].x = x_distribution(rng);
			points[i].y = y_distribution(rng);
		}

		assert(out.width  <= size_t(std::numeric_limits<int>::max));
		assert(out.height <= size_t(std::numeric_limits<int>::max));

		const int width(out.width);
		const int height(out.height);

		jcv_diagram diagram{};
		jcv_point dimensions{jcv_real(width), jcv_real(height)};
		jcv_rect rect{{0, 0}, dimensions};

		jcv_diagram_generate(count, points.get(), &rect, nullptr, &diagram);

		const jcv_site *sites = jcv_diagram_get_sites(&diagram);
		for (int i = 0; i < diagram.numsites; ++i) {
			const jcv_site &site = sites[i];
			jcv_point s = remap(site.p, diagram.min, diagram.max, dimensions);
			const T color = generator();

			const jcv_graphedge *e = site.edges;
			while (e) {
				jcv_point p0 = remap(e->pos[0], diagram.min, diagram.max, dimensions);
				jcv_point p1 = remap(e->pos[1], diagram.min, diagram.max, dimensions);
				draw_triangle(s, p0, p1, color);
				e = e->next;
			}
		}
	}
}
