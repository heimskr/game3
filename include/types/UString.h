#pragma once

#include <glibmm/ustring.h>

#include <vector>

namespace Game3 {
	class TextRenderer;

	class UString: public Glib::ustring {
		public:
			using Glib::ustring::ustring;

			UString(Glib::ustring &&) noexcept;
			UString & operator=(Glib::ustring &&) noexcept;

			std::vector<UString> split(const UString &delimiter, Glib::ustring::size_type(UString::*finder)(const Glib::ustring &, Glib::ustring::size_type) const = &Glib::ustring::find) const;
			UString wrap(const TextRenderer &, float max_width, float text_scale) const;
	};
}

template <>
struct std::formatter<Glib::ustring> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &string, auto &ctx) const {
		return std::format_to(ctx.out(), "{}", string.raw());
	}
};

template <>
struct std::formatter<Game3::UString> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &string, auto &ctx) const {
		return std::format_to(ctx.out(), "{}", string.raw());
	}
};
