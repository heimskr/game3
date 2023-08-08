#pragma once

#include <bit>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <optional>
#include <ostream>
#include <ranges>
#include <span>
#include <vector>

#include "util/Concepts.h"

namespace Game3 {
	class Buffer;

	template <typename T>
	T popBuffer(Buffer &);

	template <typename T>
	void popBuffer(Buffer &buffer, T &item) {
		item = popBuffer<T>(buffer);
	}

	template <typename C>
	std::string hexString(const C &);

	struct BufferContext {
		virtual ~BufferContext() = default;
	};

	class Buffer {
		public:
			std::vector<uint8_t> bytes;
			size_t skip = 0;

			std::span<const uint8_t> getSpan() const {
				return std::span(bytes.begin() + skip, bytes.size() - skip);
			}

			template <typename T>
			Buffer & appendType(const T &t) {
				const auto type = getType(t);
				bytes.insert(bytes.end(), type.begin(), type.end());
				return *this;
			}

			/**
			 * 0x01 -> bool / uint8_t
			 * 0x02 -> uint16_t
			 * 0x03 -> uint32_t
			 * 0x04 -> uint64_t
			 * 0x05 -> int8_t
			 * 0x06 -> int16_t
			 * 0x07 -> int32_t
			 * 0x08 -> int64_t
			 * 0x09 -> float
			 * 0x0a -> double
			 * 0x0b . type -> optional
			 * 0x0c -> optional (empty)
			 * 0x10 -> empty string
			 * [0x11, 0x1f) -> string of length [0x1, 0xf)
			 * 0x1f . (u32) length -> string of arbitrary length
			 * 0x20 . type -> linear container
			 * 0x21 . type[key] . type[value] -> map container
			 * 0xe0 -> ItemStack
			 * 0xe1 -> Inventory
			 */
			template <typename T>
			std::string getType(const T &);

			// Inelegant how it requires an instance of the type as an argument.
			template <Map M>
			std::string getType(const M &) {
				return '\x21' + getType(typename M::key_type()) + getType(typename M::mapped_type());
			}

			// Same here.
			template <LinearOrSet T>
			std::string getType(const T &) {
				return '\x20' + getType(typename T::value_type());
			}

			template <typename T>
			requires std::is_enum_v<T>
			std::string getType(const T &) {
				return getType(std::underlying_type_t<T>());
			}

			template <typename T>
			std::string getType(const std::optional<T> &item) {
				return item.has_value()? "\x0b" : "\x0c";
			}

			Buffer & operator+=(bool);
			Buffer & operator+=(char);
			Buffer & operator+=(uint8_t);
			Buffer & operator+=(uint16_t);
			Buffer & operator+=(uint32_t);
			Buffer & operator+=(uint64_t);
			Buffer & operator+=(float);
			Buffer & operator+=(double);
			Buffer & operator+=(std::string_view);

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
				assert(item);
				return *this += *item;
			}

			std::string popType();

			template <typename T1, typename T2>
			inline T2 popConv() {
				return static_cast<T2>(popBuffer<T1>(*this));
			}

			static bool typesMatch(std::string_view, std::string_view);

		public:
			std::weak_ptr<BufferContext> context;

			Buffer() = default;
			Buffer(std::weak_ptr<BufferContext> context_):
				context(std::move(context_)) {}

			template <typename... Args>
			Buffer(Args &&...args) {
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

			void popMany(size_t);
			void limitTo(size_t);
			void debug() const;

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
				return appendType(container) += container;
			}

			template <Set S>
			Buffer & operator<<(const S &set) {
				return appendType(set) += set;
			}

			template <Map M>
			Buffer & operator<<(const M &map) {
				return appendType(map) += map;
			}

			template <typename T>
			requires std::is_enum_v<T>
			Buffer & operator<<(T item) {
				using underlying = std::underlying_type_t<T>;
				return appendType(underlying{}) += static_cast<underlying>(item);
			}

			template <typename T>
			Buffer & operator<<(const std::optional<T> &item) {
				return *this += item;
			}

			template <typename T>
			Buffer & operator<<(const std::shared_ptr<T> &item) {
				assert(item);
				return *this += *item;
			}

			friend std::ostream & operator<<(std::ostream &, const Buffer &);

			template <typename T>
			Buffer & operator>>(T &);

			template <LinearOrSet T>
			Buffer & operator>>(T &out) {
				const auto type = popType();
				if (!typesMatch(type, getType(T()))) {
					debug();
					throw std::invalid_argument("Invalid type in buffer (expected list: " + hexString(getType(T())) + "): " + hexString(type));
				}
				out = popBuffer<T>(*this);
				return *this;
			}

			template <Map M>
			Buffer & operator>>(M &out) {
				const auto type = popType();
				if (!typesMatch(type, getType(M()))) {
					debug();
					throw std::invalid_argument("Invalid type in buffer (expected map: " + hexString(getType(M())) + "): " + hexString(type));
				}
				out = popBuffer<M>(*this);
				return *this;
			}

			template <Numeric T>
			Buffer & operator>>(T &out) {
				const auto type = popType();
				if (!typesMatch(type, getType(T()))) {
					debug();
					throw std::invalid_argument("Invalid type in buffer (expected integral: " + hexString(getType(T())) + "): " + hexString(type));
				}
				out = popBuffer<T>(*this);
				return *this;
			}

			template <typename T>
			requires std::is_enum_v<T>
			Buffer & operator>>(T &out) {
				std::underlying_type_t<T> raw;
				*this >> raw;
				out = static_cast<T>(raw);
				return *this;
			}

			template <typename T>
			Buffer & operator>>(std::optional<T> &out) {
				const auto type = popType();
				if (!typesMatch(type, getType(std::make_optional<T>())))
					throw std::invalid_argument("Invalid type in buffer (expected " + std::string(typeid(T).name()) + ": " + hexString(getType(std::make_optional<T>())) + ')');
				if (type == "\x0c")
					out = std::nullopt;
				else
					out = take<T>();
				return *this;
			}

			template <typename T>
			Buffer & operator>>(std::shared_ptr<T> &out) {
				assert(out);
				return *this >> *out;
			}

			template <typename T>
			T take() {
				T out;
				*this >> out;
				return out;
			}

			inline auto str() const {
				return std::string(bytes.begin() + skip, bytes.end());
			}

			template <typename T>
			friend T popBuffer(Buffer &);
	};

	template <std::integral T>
	T popBuffer(Buffer &buffer) {
		std::span span = buffer.getSpan();

		if (span.size_bytes() < sizeof(T))
			throw std::out_of_range("Buffer is too empty");

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
		for (uint32_t i = 0; i < size; ++i)
			out.emplace(popBuffer<typename M::key_type>(buffer), popBuffer<typename M::mapped_type>(buffer));
		return out;
	}

	template <typename T>
	requires std::is_enum_v<T>
	T popBuffer(Buffer &buffer) {
		return static_cast<T>(popBuffer<std::underlying_type_t<T>>(buffer));
	}

	std::ostream & operator<<(std::ostream &, const Buffer &);
}
