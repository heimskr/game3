#pragma once

namespace Game3 {
	template <typename Src, typename Dest>
	Dest convertColor(const Src &);

	template <typename T>
	struct BaseColor {
		template <typename U>
		U convert() {
			return convertColor<T, U>(static_cast<const T &>(*this));
		}
	};

	struct OKHsv: BaseColor<OKHsv> {
		float h{};
		float s{};
		float v{};

		OKHsv(float h_, float s_, float v_):
			h(h_), s(s_), v(v_) {}
	};

	struct RGB: BaseColor<RGB> {
		float r{};
		float g{};
		float b{};

		RGB(float r_, float g_, float b_):
			r(r_), g(g_), b(b_) {}
	};
}
