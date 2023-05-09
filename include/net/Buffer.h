#pragma once

#include <bit>
#include <concepts>
#include <cstdint>
#include <list>
#include <ostream>
#include <ranges>
#include <span>
#include <vector>

#include "util/Concepts.h"

namespace Game3 {
	class Buffer {
		private:
			std::vector<uint8_t> bytes;

			template <typename T>
			Buffer & appendType(const T &);

			// Inelegant how it requires an instance of the type as an argument.
			template <map_type M>
			Buffer & appendType(const M &) {
				return append('\x21').appendType(typename M::key_type()).appendType(typename M::mapped_type());
			}

			// Same here.
			template <typename T>
			Buffer & appendType(const std::vector<T> &) {
				return append('\x20').appendType(T());
			}

			// And here.
			template <typename T>
			Buffer & appendType(const std::list<T> &) {
				return append('\x20').appendType(T());
			}

			Buffer & append(char);
			Buffer & append(uint8_t);
			Buffer & append(uint16_t);
			Buffer & append(uint32_t);
			Buffer & append(uint64_t);
			Buffer & append(std::string_view);

			template <map_type M>
			Buffer & append(const M &map) {
				for (const auto &[key, value]: map)
					append(key).append(value);
				return *this;
			}

			template <typename T>
			Buffer & append(const std::vector<T> &vector) {
				for (const auto &item: vector)
					append(item);
				return *this;
			}

			template <typename T>
			Buffer & append(const std::list<T> &list) {
				for (const auto &item: list)
					append(item);
				return *this;
			}

		public:
			Buffer() = default;

			std::span<const uint8_t> getSpan() const;
			inline explicit operator std::span<const uint8_t>() const { return getSpan(); }

			Buffer & operator<<(uint8_t);
			Buffer & operator<<(uint16_t);
			Buffer & operator<<(uint32_t);
			Buffer & operator<<(uint64_t);
			Buffer & operator<<(int8_t);
			Buffer & operator<<(int16_t);
			Buffer & operator<<(int32_t);
			Buffer & operator<<(int64_t);
			Buffer & operator<<(std::string_view);

			template <map_type M>
			Buffer & operator<<(const M &map) {
				return appendType(map).append(map);
			}

			template <typename T>
			Buffer & operator<<(const std::vector<T> &vector) {
				return appendType(vector).append(vector);
			}

			template <typename T>
			Buffer & operator<<(const std::list<T> &list) {
				return appendType(list).append(list);
			}

			friend std::ostream & operator<<(std::ostream &, const Buffer &);
	};

	std::ostream & operator<<(std::ostream &, const Buffer &);
}
