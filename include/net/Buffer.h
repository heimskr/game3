#pragma once

#include "Log.h"
#include "util/Concepts.h"
#include "util/Demangle.h"

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

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class Buffer;

	template <typename T>
	T popBuffer(Buffer &);

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
			Buffer & appendType(const T &t) {
				const auto type = getType(t);
				bytes.insert(bytes.end(), type.begin(), type.end());
				return *this;
			}

			/* See doc/Protocol.md for a list of standard types. */
			template <typename T>
			std::string getType(const T &);

			template <Map M>
			std::string getType(const M &) {
				// Inelegant how it requires an instance of the type as an argument.
				return '\x21' + getType(typename M::key_type()) + getType(typename M::mapped_type());
			}

			template <LinearOrSet T>
			std::string getType(const T &) {
				// Same here.
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

			Buffer & operator+=(std::same_as<bool> auto);
			Buffer & operator+=(std::same_as<uint8_t> auto);
			Buffer & operator+=(std::same_as<uint16_t> auto);
			Buffer & operator+=(std::same_as<uint32_t> auto);
			Buffer & operator+=(std::same_as<uint64_t> auto);
			Buffer & operator+=(std::same_as<float> auto);
			Buffer & operator+=(std::same_as<double> auto);
			Buffer & operator+=(std::same_as<std::string_view> auto);
			Buffer & operator+=(std::same_as<std::string> auto const &);
			Buffer & operator+=(std::same_as<nlohmann::json> auto const &);

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

			template <typename T1, typename T2>
			inline T2 popConv() {
				return static_cast<T2>(popBuffer<T1>(*this));
			}

			static bool typesMatch(std::string_view, std::string_view);

		public:
			std::weak_ptr<BufferContext> context;

			Buffer() = default;

			Buffer(const Buffer &) = default;
			Buffer(Buffer &&);

			Buffer & operator=(const Buffer &) = default;
			Buffer & operator=(Buffer &&);

			explicit Buffer(std::weak_ptr<BufferContext> context_):
				context(std::move(context_)) {}

			explicit Buffer(decltype(bytes) bytes_):
				bytes(std::move(bytes_)) {}

			template <typename... Args>
			explicit Buffer(Args &&...args) {
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
				if (item)
					return (*this += '\x0b') << *item;
				return *this += '\x0c';
			}

			friend std::ostream & operator<<(std::ostream &, const Buffer &);

			template <typename T>
			Buffer & operator>>(T &);

			template <LinearOrSet T>
			Buffer & operator>>(T &out) {
				const auto type = popType();
				if (!typesMatch(type, getType(T()))) {
					debug();
					throw std::invalid_argument("Invalid type in buffer (expected list: " + hexString(getType(T()), true) + "): " + hexString(type, true));
				}
				out = popBuffer<T>(*this);
				return *this;
			}

			template <Map M>
			Buffer & operator>>(M &out) {
				const auto type = popType();
				if (!typesMatch(type, getType(M()))) {
					debug();
					throw std::invalid_argument("Invalid type in buffer (expected map: " + hexString(getType(M()), true) + "): " + hexString(type, true));
				}
				out = popBuffer<M>(*this);
				return *this;
			}

			template <Numeric T>
			Buffer & operator>>(T &out) {
				const auto type = popType();
				if (!typesMatch(type, getType(T()))) {
					debug();
					throw std::invalid_argument("Invalid type in buffer (expected integral: " + hexString(getType(T()), true) + "): " + hexString(type, true));
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
				if (!typesMatch(type, getType(std::optional<T>())))
					throw std::invalid_argument("Invalid type in buffer (expected optional<" + DEMANGLE(T) + ">: " + hexString(getType(std::make_optional<T>()), true) + "): " + hexString(type, true));
				if (type == "\x0c")
					out = std::nullopt;
				else
					out = take<T>();
				return *this;
			}

			template <typename T>
			Buffer & operator>>(std::shared_ptr<T> &out) {
				const auto type = popType();
				if (!typesMatch(type, getType(std::optional<T>())))
					throw std::invalid_argument("Invalid type in buffer (expected optional<" + DEMANGLE(T) + ">: " + hexString(getType(std::make_optional<T>()), true) + "): " + hexString(type, true));
				if (type == "\x0c") {
					out = {};
				} else {
					if (!out)
						out = std::make_shared<T>();
					*this >> *out;
				}
				return *this;
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

		if (span.size_bytes() < sizeof(T)) {
			ERROR("Buffer size: " << buffer.bytes.size());
			ERROR("Skip: " << buffer.skip);
			ERROR("Span size: " << span.size());
			ERROR("Span size_bytes: " << span.size_bytes());
			ERROR("sizeof(" << typeid(T).name() << "): " << sizeof(T));
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

	std::ostream & operator<<(std::ostream &, const Buffer &);

	Buffer & operator<<(Buffer &, std::same_as<nlohmann::json> auto const &);
	Buffer & operator>>(Buffer &, std::same_as<nlohmann::json> auto &);
}
