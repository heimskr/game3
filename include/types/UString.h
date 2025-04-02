#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#include <glibmm/ustring.h>
#pragma GCC diagnostic pop

#include <format>
#include <vector>

namespace Game3 {
	class TextRenderer;
	class UStringSpan;

	class UString: public Glib::ustring {
		public:
			using Glib::ustring::ustring;

			UString(Glib::ustring &&) noexcept;
			UString & operator=(Glib::ustring &&) noexcept;

			std::vector<UString> split(const UString &delimiter, Glib::ustring::size_type(UString::*finder)(const Glib::ustring &, Glib::ustring::size_type) const = &Glib::ustring::find) const;
			UString wrap(const TextRenderer &, float max_width, float text_scale) const;

		private:
			std::vector<UStringSpan> getLines() const;

		friend class UStringTest;
	};

	class UStringSpan {
		public:
			using iterator = Glib::ustring::const_iterator;

			iterator left;
			iterator right;

			UStringSpan(iterator left, iterator right);

			explicit operator UString() const;
			explicit operator std::string() const;

			bool operator==(const UStringSpan &) const;
			bool operator==(const UString &) const;

			bool empty() const;
			std::size_t size_bytes() const;

			iterator begin() const { return left; }
			iterator end() const { return right; }
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
