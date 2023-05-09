#pragma once

#include <bit>
#include <concepts>
#include <cstdint>
#include <ranges>
#include <span>
#include <vector>

#include "util/Concepts.h"

namespace Game3 {
	class Buffer {
		private:
			std::vector<uint8_t> bytes;

			Buffer & appendType(uint8_t)  { return append('\x01'); }
			Buffer & appendType(uint16_t) { return append('\x02'); }
			Buffer & appendType(uint32_t) { return append('\x03'); }
			Buffer & appendType(uint64_t) { return append('\x04'); }
			Buffer & appendType(int8_t)   { return append('\x05'); }
			Buffer & appendType(int16_t)  { return append('\x06'); }
			Buffer & appendType(int32_t)  { return append('\x07'); }
			Buffer & appendType(int64_t)  { return append('\x08'); }
			Buffer & appendType(std::string_view);

			// Inelegant how it requires an instance of the type as an argument.
			template <map_type M>
			Buffer & appendType(const M &) {
				return append('\x21').appendType(M::key_type()).appendType(M::mapped_type());
			}

			// Same here.
			template <std::ranges::range R>
			Buffer & appendType(const R &) {
				return append('\x20').appendType(R::value_type());
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

			template <std::ranges::range R>
			Buffer & operator<<(const R &range) {
				return appendType(range).append(range);
			}
	};
}
