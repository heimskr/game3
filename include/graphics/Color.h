#pragma once

#include <string_view>

namespace Game3 {
	template <typename Src, typename Dest>
	Dest convertColor(const Src &);

	template <typename T>
	struct BaseColor {
		template <typename U>
		U convert() const {
			return convertColor<T, U>(static_cast<const T &>(*this));
		}
	};

	struct OKHsv: BaseColor<OKHsv> {
		float h{};
		float s{};
		float v{};

		OKHsv(float h, float s, float v):
			h(h), s(s), v(v) {}

		OKHsv darken() const;
	};

	struct RGB: BaseColor<RGB> {
		float r{};
		float g{};
		float b{};

		RGB(float r, float g, float b):
			r(r), g(g), b(b) {}

		explicit RGB(std::string_view);

		RGB darken() const;
	};
}
