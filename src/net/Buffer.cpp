#include <cassert>
#include <cstring>
#include <iomanip>

#include "net/Buffer.h"

namespace Game3 {
	template <> Buffer & Buffer::appendType<uint8_t> (const uint8_t &)  { return append('\x01'); }
	template <> Buffer & Buffer::appendType<uint16_t>(const uint16_t &) { return append('\x02'); }
	template <> Buffer & Buffer::appendType<uint32_t>(const uint32_t &) { return append('\x03'); }
	template <> Buffer & Buffer::appendType<uint64_t>(const uint64_t &) { return append('\x04'); }
	template <> Buffer & Buffer::appendType<char>    (const char &)     { return append('\x05'); }
	template <> Buffer & Buffer::appendType<int8_t>  (const int8_t &)   { return append('\x05'); }
	template <> Buffer & Buffer::appendType<int16_t> (const int16_t &)  { return append('\x06'); }
	template <> Buffer & Buffer::appendType<int32_t> (const int32_t &)  { return append('\x07'); }
	template <> Buffer & Buffer::appendType<int64_t> (const int64_t &)  { return append('\x08'); }

	template <>
	Buffer & Buffer::appendType<std::string_view>(const std::string_view &string) {
		if (string.size() == 0)
			return append('\x10');

		if (string.size() < 0xf)
			return append(static_cast<char>('\x10' + string.size()));

		assert(string.size() <= UINT32_MAX);
		return append('\x1f').append(static_cast<uint32_t>(string.size()));
	}

	template <>
	Buffer & Buffer::appendType<std::string>(const std::string &string) {
		return appendType(std::string_view(string));
	}

	Buffer & Buffer::append(char item) {
		bytes.insert(bytes.end(), static_cast<uint8_t>(item));
		return *this;
	}

	Buffer & Buffer::append(uint8_t item) {
		bytes.insert(bytes.end(), item);
		return *this;
	}

	Buffer & Buffer::append(uint16_t item) {
		if (std::endian::native == std::endian::little) {
			const auto *raw = reinterpret_cast<const uint8_t *>(&item);
			bytes.insert(bytes.end(), raw, raw + sizeof(item));
		} else
			bytes.insert(bytes.end(), {static_cast<uint8_t>(item), static_cast<uint8_t>(item >> 8)});
		return *this;
	}

	Buffer & Buffer::append(uint32_t item) {
		if (std::endian::native == std::endian::little) {
			const auto *raw = reinterpret_cast<const uint8_t *>(&item);
			bytes.insert(bytes.end(), raw, raw + sizeof(item));
		} else
			bytes.insert(bytes.end(), {static_cast<uint8_t>(item), static_cast<uint8_t>(item >> 8), static_cast<uint8_t>(item >> 16), static_cast<uint8_t>(item >> 24)});
		return *this;
	}

	Buffer & Buffer::append(uint64_t item) {
		if (std::endian::native == std::endian::little) {
			const auto *raw = reinterpret_cast<const uint8_t *>(&item);
			bytes.insert(bytes.end(), raw, raw + sizeof(item));
		} else {
			bytes.insert(bytes.end(), {
				static_cast<uint8_t>(item),
				static_cast<uint8_t>(item >> 8),
				static_cast<uint8_t>(item >> 16),
				static_cast<uint8_t>(item >> 24),
				static_cast<uint8_t>(item >> 32),
				static_cast<uint8_t>(item >> 40),
				static_cast<uint8_t>(item >> 48),
				static_cast<uint8_t>(item >> 56),
			});
		}
		return *this;
	}

	Buffer & Buffer::append(std::string_view string) {
		bytes.insert(bytes.end(), string.begin(), string.end());
		return *this;
	}

	Buffer & Buffer::operator<<(uint8_t item) {
		return appendType(item).append(item);
	}

	Buffer & Buffer::operator<<(uint16_t item) {
		return appendType(item).append(item);
	}

	Buffer & Buffer::operator<<(uint32_t item) {
		return appendType(item).append(item);
	}

	Buffer & Buffer::operator<<(uint64_t item) {
		return appendType(item).append(item);
	}

	Buffer & Buffer::operator<<(int8_t item) {
		return appendType(item).append(static_cast<uint8_t>(item));
	}

	Buffer & Buffer::operator<<(int16_t item) {
		return appendType(item).append(static_cast<uint16_t>(item));
	}

	Buffer & Buffer::operator<<(int32_t item) {
		return appendType(item).append(static_cast<uint32_t>(item));
	}

	Buffer & Buffer::operator<<(int64_t item) {
		return appendType(item).append(static_cast<uint64_t>(item));
	}

	Buffer & Buffer::operator<<(std::string_view string) {
		return appendType(string).append(string);
	}

	std::ostream & operator<<(std::ostream &os, const Buffer &buffer) {
		os << "Buffer<";

		for (bool first = true; const uint16_t byte: buffer.bytes) {
			if (first)
				first = false;
			else
				os << ' ';
			os << std::hex << std::setw(2) << std::setfill('0') << std::right << byte;
		}

		return os << '>';
	}
}
