#pragma once

namespace Game3 {
	template <typename Src, typename Dest>
	Dest convertColor(const Src &);

	template <typename T>
	struct Color {
		template <typename U>
		U convert() {
			return convertColor<U>(*this);
		}
	};

	struct OKHsv: Color<OKHsv> {
		float h{};
		float s{};
		float v{};

		OKHsv(float h_, float s_, float v_):
			h(h_), s(s_), v(v_) {}
	};

	struct RGB: Color<RGB> {
		float r{};
		float g{};
		float b{};

		RGB(float r_, float g_, float b_):
			r(r_), g(g_), b(b_) {}
	};
}
