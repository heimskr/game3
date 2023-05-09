#include <cassert>
#include <cstring>

#include "net/Buffer.h"

namespace Game3 {
	std::span<const uint8_t> Buffer::getSpan() const {
		return {bytes.cbegin(), bytes.size()};
	}

	Buffer & Buffer::appendType(std::string_view string) {
		if (string.size() == 0)
			return append('\x10');

		if (string.size() < 0xf)
			return append(static_cast<char>('\x10' + string.size()));

		assert(string.size() <= UINT32_MAX);
		return append('\x1f').append(static_cast<uint32_t>(string.size()));
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
}
