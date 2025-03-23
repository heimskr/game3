#pragma once

#include "util/Log.h"
#include "types/Types.h"
#include "util/Concepts.h"
#include "util/Demangle.h"

#include <boost/json/fwd.hpp>

#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <deque>
#include <iomanip>
#include <iostream>
#include <list>
#include <memory>
#include <optional>
#include <ostream>
#include <ranges>
#include <span>
#include <vector>

namespace Game3 {
	class Buffer;

	template <typename T>
	T popBuffer(Buffer &);

	template <typename T>
	T makeForBuffer(Buffer &) {
		return T();
	}

	template <typename T>
	void popBuffer(Buffer &buffer, T &item) {
		item = popBuffer<T>(buffer);
	}

	template <typename C>
	std::string hexString(const C &, bool);

	struct BufferContext {
		virtual ~BufferContext() = default;
	};

	class Buffer {
		public:
			std::vector<uint8_t> bytes;
			size_t skip = 0;

			std::span<const uint8_t> getSpan() const {
				return std::span(bytes.data() + skip, bytes.size() - skip);
			}

			template <typename T>
			Buffer & appendType(const T &t, bool in_container) {
				const auto type = getType(t, in_container);
				bytes.insert(bytes.end(), type.begin(), type.end());
				return *this;
			}

			/* See doc/Protocol.md for a list of standard types. */
			template <typename T>
			std::string getType(const T &, bool in_container);

			template <Map M>
			std::string getType(const M &, bool in_container) {
				(void) in_container;
				// Inelegant how it requires an instance of the type as an argument.
				return '\x21' + getType(typename M::key_type(), true) + getType(typename M::mapped_type(), true);
			}

			template <LinearOrSet T>
			std::string getType(const T &, bool in_container) {
				(void) in_container;
				// Same here.
				return '\x20' + getType(typename T::value_type(), true);
			}

			template <typename T>
			requires std::is_enum_v<T>
			std::string getType(const T &, bool in_container) {
				return getType(std::underlying_type_t<T>(), in_container);
			}

			template <typename T>
			std::string getType(const std::optional<T> &item, bool in_container) {
				if (in_container)
					return "\x0b" + getType(T(), true);
				return item.has_value()? "\x0b" : "\x0c";
			}

			template <typename T>
			std::string getType(const std::shared_ptr<T> &item, bool in_container) {
				if (!item)
					return getType(T(), in_container);
				return getType(*item, in_container);
			}

			Buffer & operator+=(std::same_as<bool> auto);
			Buffer & operator+=(std::same_as<uint8_t> auto);
			Buffer & operator+=(std::same_as<uint16_t> auto);
			Buffer & operator+=(std::same_as<uint32_t> auto);
			Buffer & operator+=(std::same_as<uint64_t> auto);
			Buffer & operator+=(std::same_as<float> auto);
			Buffer & operator+=(std::same_as<double> auto);
			Buffer & operator+=(std::same_as<std::string_view> auto);
			Buffer & operator+=(std::same_as<std::string> auto const &);
			Buffer & operator+=(std::same_as<boost::json::value> auto const &);

			template <std::signed_integral T>
			Buffer & operator+=(T item) {
				return *this += static_cast<std::make_unsigned_t<T>>(item);
			}

			template <typename T>
			requires std::is_enum_v<T>
			Buffer & operator+=(T item) {
				return *this += static_cast<std::underlying_type_t<T>>(item);
			}

			template <LinearOrSet T>
			Buffer & operator+=(const T &container) {
				assert(container.size() <= UINT32_MAX);
				*this += static_cast<uint32_t>(container.size());
				for (const auto &item: container)
					*this += item;
				return *this;
			}

			template <Map M>
			Buffer & operator+=(const M &map) {
				assert(map.size() <= UINT32_MAX);
				*this += static_cast<uint32_t>(map.size());
				for (const auto &[key, value]: map) {
					*this += key;
					*this += value;
				}
				return *this;
			}

			template <typename T>
			Buffer & operator+=(const std::optional<T> &item) {
				if (item.has_value())
					return (*this += '\x0b') << *item;
				return *this += '\x0c';
			}

			template <typename T>
			Buffer & operator+=(const std::shared_ptr<T> &item) {
				return *this << item;
			}

			std::string popType();
			std::string peekType(size_t to_skip = 0);

			template <typename T1, typename T2>
			inline T2 popConv() {
				return static_cast<T2>(popBuffer<T1>(*this));
			}

			template <typename T>
			T peek(size_t to_skip) const;

			template <std::integral T>
			T peek(size_t to_skip) const {
				std::span span = getSpan().subspan(to_skip);

				if (span.size_bytes() < sizeof(T)) {
					ERR("Buffer size: {:L}", bytes.size());
					ERR("Skip: {:L}", skip);
					ERR("Span size: {:L}", span.size());
					ERR("Span size_bytes: {:L}", span.size_bytes());
					ERR("sizeof({}): {}", DEMANGLE(T), sizeof(T));
					throw std::out_of_range("Buffer is too empty");
				}

				T out{};
				std::memmove(reinterpret_cast<char *>(&out), span.data(), sizeof(T));

				if constexpr (std::endian::native == std::endian::big)
					return swapBytes(out);
				return out;
			}

			static bool typesMatch(std::string_view, std::string_view);

		public:
			Side target;
			std::weak_ptr<BufferContext> context;

			Buffer() = delete;
			explicit Buffer(Side target);
			Buffer(std::vector<uint8_t> bytes, std::weak_ptr<BufferContext>, Side target);
			Buffer(std::weak_ptr<BufferContext>, Side target);
			Buffer(std::vector<uint8_t> bytes, Side target);

			Buffer(const Buffer &) = default;
			Buffer(Buffer &&) noexcept;

			Buffer & operator=(const Buffer &) = default;
			Buffer & operator=(Buffer &&) noexcept;

			template <typename... Args>
			explicit Buffer(Side target, Args &&...args): target(target) {
				(void) std::initializer_list<int> {
					((void) (*this << std::forward<Args>(args)), 0)...
				};
			}

			inline auto size() const { return bytes.size() - skip; }
			inline bool empty() const { return bytes.size() == skip; }
			inline void clear() { bytes.clear(); skip = 0; }
			inline void reserve(size_t to_reserve) { bytes.reserve(to_reserve); }
			inline auto & getBytes() { return bytes; }
			inline const auto & getBytes() const { return bytes; }

			template <typename It>
			void append(It first, It last) {
				bytes.insert(bytes.end(), first, last);
			}

			template <typename T>
			void append(const T &range) {
				bytes.insert(bytes.end(), range.begin(), range.end());
			}

			void popMany(size_t);
			void limitTo(size_t);
			void debug() const;
			size_t getSkip() const;
			void setSkip(size_t);

			boost::json::value popJSON();
			boost::json::value popAllJSON();

			Buffer & operator<<(bool);
			Buffer & operator<<(uint8_t);
			Buffer & operator<<(uint16_t);
			Buffer & operator<<(uint32_t);
			Buffer & operator<<(uint64_t);
			Buffer & operator<<(int8_t);
			Buffer & operator<<(int16_t);
			Buffer & operator<<(int32_t);
			Buffer & operator<<(int64_t);
			Buffer & operator<<(float);
			Buffer & operator<<(double);
			Buffer & operator<<(std::string_view);
			Buffer & operator<<(const std::string &);
			Buffer & operator<<(const char *);
			Buffer & operator<<(const Buffer &);

			template <LinearOrSet T>
			Buffer & operator<<(const T &container) {
				return appendType(container, false) += container;
			}

			template <Map M>
			Buffer & operator<<(const M &map) {
				return appendType(map, false) += map;
			}

			template <typename T>
			requires std::is_enum_v<T>
			Buffer & operator<<(T item) {
				using underlying = std::underlying_type_t<T>;
				return appendType(underlying{}, false) += static_cast<underlying>(item);
			}

			template <typename T>
			Buffer & operator<<(const std::optional<T> &item) {
				return *this += item;
			}

			template <typename T>
			Buffer & operator<<(const std::shared_ptr<T> &item) {
				if (item)
					return (*this += '\x0b') << *item;
				return *this += '\x0c';
			}

			template <typename T>
			T take() {
				T out = makeForBuffer<T>(*this);
				*this >> out;
				return out;
			}

			inline auto str() const {
				return std::string(bytes.begin() + skip, bytes.end());
			}

			template <typename T>
			friend T popBuffer(Buffer &);
	};

	template <typename T>
	Buffer & operator>>(Buffer &, T &);

	template <LinearOrSet T>
	Buffer & operator>>(Buffer &buffer, T &out) {
		const auto type = buffer.popType();
		if (!buffer.typesMatch(type, buffer.getType(T(), false))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type in buffer (expected list: " + hexString(buffer.getType(T(), false), true) + "): " + hexString(type, true));
		}
		out = popBuffer<T>(buffer);
		return buffer;
	}

	template <Map M>
	Buffer & operator>>(Buffer &buffer, M &out) {
		const auto type = buffer.popType();
		if (!buffer.typesMatch(type, buffer.getType(M(), false))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type in buffer (expected map: " + hexString(buffer.getType(M(), false), true) + "): " + hexString(type, true));
		}
		out = popBuffer<M>(buffer);
		return buffer;
	}

	template <Numeric T>
	Buffer & operator>>(Buffer &buffer, T &out) {
		const auto type = buffer.popType();
		if (!buffer.typesMatch(type, buffer.getType(T(), false))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type in buffer (expected integral: " + hexString(buffer.getType(T(), false), true) + "): " + hexString(type, true));
		}
		out = popBuffer<T>(buffer);
		return buffer;
	}

	template <typename T>
	requires std::is_enum_v<T>
	Buffer & operator>>(Buffer &buffer, T &out) {
		std::underlying_type_t<T> raw;
		buffer >> raw;
		out = static_cast<T>(raw);
		return buffer;
	}

	template <typename T>
	Buffer & operator>>(Buffer &buffer, std::optional<T> &out) {
		const auto type = buffer.popType();
		if (!buffer.typesMatch(type, buffer.getType(std::optional<T>(), false)))
			throw std::invalid_argument("Invalid type in buffer (expected optional<" + DEMANGLE(T) + ">: " + hexString(buffer.getType(std::make_optional<T>(), false), true) + "): " + hexString(type, true));
		if (type == "\x0c")
			out = std::nullopt;
		else
			out = buffer.take<T>();
		return buffer;
	}

	template <typename T>
	Buffer & operator>>(Buffer &buffer, std::shared_ptr<T> &out) {
		const auto type = buffer.popType();
		if (!buffer.typesMatch(type, buffer.getType(std::optional<T>(), false)))
			throw std::invalid_argument("Invalid type in buffer (expected optional<" + DEMANGLE(T) + ">: " + hexString(buffer.getType(std::optional<T>(), true), true) + "): " + hexString(type, true));
		if (type == "\x0c") {
			out = {};
		} else {
			if (!out)
				out = std::make_shared<T>();
			buffer >> *out;
		}
		return buffer;
	}

	using BufferPtr = std::shared_ptr<Buffer>;

	template <std::integral T>
	T popBuffer(Buffer &buffer) {
		std::span span = buffer.getSpan();

		if (span.size_bytes() < sizeof(T)) {
			ERR("Buffer size: {:L}", buffer.bytes.size());
			ERR("Skip: {:L}", buffer.skip);
			ERR("Span size: {:L}", span.size());
			ERR("Span size_bytes: {:L}", span.size_bytes());
			ERR("sizeof({}): {}", DEMANGLE(T), sizeof(T));
			throw std::out_of_range("Buffer is too empty");
		}

		T out{};
		std::memmove(reinterpret_cast<char *>(&out), span.data(), sizeof(T));

		buffer.skip += sizeof(T);

		if constexpr (std::endian::native == std::endian::big)
			return swapBytes(out);
		return out;
	}

	template <Linear C>
	C popBuffer(Buffer &buffer) {
		const auto size = popBuffer<uint32_t>(buffer);
		C out;
		if constexpr (Reservable<C>)
			out.reserve(size);

		for (uint32_t i = 0; i < size; ++i)
			out.push_back(popBuffer<typename C::value_type>(buffer));

		return out;
	}

	template <Set S>
	S popBuffer(Buffer &buffer) {
		const auto size = popBuffer<uint32_t>(buffer);
		S out;
		for (uint32_t i = 0; i < size; ++i)
			out.insert(popBuffer<typename S::value_type>(buffer));
		return out;
	}

	template <Map M>
	M popBuffer(Buffer &buffer) {
		const auto size = popBuffer<uint32_t>(buffer);
		M out;
		for (uint32_t i = 0; i < size; ++i) {
			auto key = popBuffer<typename M::key_type>(buffer);
			auto value = popBuffer<typename M::mapped_type>(buffer);
			out.emplace(std::move(key), std::move(value));
		}
		return out;
	}

	template <typename T>
	requires std::is_enum_v<T>
	T popBuffer(Buffer &buffer) {
		return static_cast<T>(popBuffer<std::underlying_type_t<T>>(buffer));
	}

	Buffer & operator<<(Buffer &, std::same_as<boost::json::value> auto const &);

	template <>
	Buffer & operator>>(Buffer &, boost::json::value &);
}

template <>
struct std::formatter<Game3::Buffer> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &buffer, auto &ctx) const {
		std::stringstream ss;
		ss << "Buffer<";

		for (bool first = true; const uint16_t byte: buffer.bytes) {
			if (first)
				first = false;
			else
				ss << ' ';
			ss << std::hex << std::setw(2) << std::setfill('0') << std::right << byte << std::dec;
		}

		ss << ">[" << buffer.size() << ']';
		return std::format_to(ctx.out(), "{}", ss.str());
	}
};
