#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#include <glibmm/ustring.h>
#pragma GCC diagnostic pop

#include <format>
#include <functional>
#include <optional>
#include <vector>

namespace Game3 {
	class Buffer;
	class TextRenderer;
	class UStringSpan;

	class UString: public Glib::ustring {
		public:
			using Glib::ustring::ustring;

			UString(Glib::ustring &&) noexcept;
			UString(const UStringSpan &);

			using Glib::ustring::operator=;
			UString & operator=(Glib::ustring &&) noexcept;
			UString & operator=(const UStringSpan &);

			using Glib::ustring::operator+=;
			UString & operator+=(const UStringSpan &);

			operator UStringSpan() const;

			std::vector<UStringSpan> split(const UString &delimiter, Glib::ustring::size_type(UString::*finder)(const Glib::ustring &, Glib::ustring::size_type) const = &Glib::ustring::find) const;
			UString wrap(const TextRenderer &, float max_width, float text_scale) const;

			UStringSpan span(size_t pos, size_t n = npos);

		private:
			std::vector<UStringSpan> getLines() const;

		friend class UStringTest;
	};

	template <typename T>
	T popBuffer(Buffer &);
	template <>
	UString popBuffer<UString>(Buffer &);
	Buffer & operator+=(Buffer &, const UString &);
	Buffer & operator<<(Buffer &, const UString &);
	Buffer & operator>>(Buffer &, UString &);

	class UStringSpan {
		public:
			using iterator = Glib::ustring::const_iterator;

			iterator left;
			iterator right;

			UStringSpan(iterator left, iterator right);

			explicit operator UString() const;
			explicit operator std::string() const;
			explicit operator std::string_view() const;

			bool operator==(const UStringSpan &) const;
			bool operator==(const UString &) const;
			bool operator==(std::string_view) const;
			bool operator==(const char *) const;

			bool empty() const;
			std::size_t size_bytes() const;
			/** Number of code points. Expensive! O(n). */
			std::size_t size() const;

			std::vector<UStringSpan> split(const UString &delimiter) const;
			void split(const UString &delimiter, const std::function<void(UStringSpan)> &) const;

			bool starts_with(const UStringSpan &) const;
			iterator find(const UStringSpan &) const;

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

template <>
struct std::formatter<Game3::UStringSpan> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &span, auto &ctx) const {
		return std::format_to(ctx.out(), "{}", static_cast<std::string_view>(span));
	}
};
