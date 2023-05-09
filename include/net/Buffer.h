#pragma once

#include <bit>
#include <concepts>
#include <cstdint>
#include <deque>
#include <list>
#include <ostream>
#include <ranges>
#include <span>
#include <vector>

#include "util/Concepts.h"

namespace Game3 {
	class Buffer {
		private:
			std::deque<uint8_t> bytes;

			template <typename T>
			Buffer & appendType(const T &);

			// Inelegant how it requires an instance of the type as an argument.
			template <Map M>
			Buffer & appendType(const M &) {
				return append('\x21').appendType(typename M::key_type()).appendType(typename M::mapped_type());
			}

			// Same here.
			template <LinearContainer T>
			Buffer & appendType(const T &) {
				return append('\x20').appendType(typename T::value_type());
			}

			Buffer & append(char);
			Buffer & append(uint8_t);
			Buffer & append(uint16_t);
			Buffer & append(uint32_t);
			Buffer & append(uint64_t);
			Buffer & append(std::string_view);

			template <Map M>
			Buffer & append(const M &map) {
				for (const auto &[key, value]: map)
					append(key).append(value);
				return *this;
			}

			template <LinearContainer T>
			Buffer & append(const T &container) {
				for (const auto &item: container)
					append(item);
				return *this;
			}

		public:
			Buffer() = default;

			Buffer & operator<<(uint8_t);
			Buffer & operator<<(uint16_t);
			Buffer & operator<<(uint32_t);
			Buffer & operator<<(uint64_t);
			Buffer & operator<<(int8_t);
			Buffer & operator<<(int16_t);
			Buffer & operator<<(int32_t);
			Buffer & operator<<(int64_t);
			Buffer & operator<<(std::string_view);

			template <Map M>
			Buffer & operator<<(const M &map) {
				return appendType(map).append(map);
			}

			template <LinearContainer T>
			Buffer & operator<<(const T &container) {
				return appendType(container).append(container);
			}

			friend std::ostream & operator<<(std::ostream &, const Buffer &);
	};

	std::ostream & operator<<(std::ostream &, const Buffer &);
}
