#include <cassert>
#include <cstring>
#include <iomanip>

#include "Log.h"
#include "net/Buffer.h"
#include "util/Util.h"

namespace Game3 {
	Buffer::Buffer(Buffer &&other):
	bytes(std::move(other.bytes)), skip(other.skip), context(std::move(other.context)) {
		other.skip = 0;
	}

	Buffer & Buffer::operator=(Buffer &&other) {
		bytes = std::move(other.bytes);
		skip = other.skip;
		context = std::move(other.context);
		other.skip = 0;
		return *this;
	}

	template <> std::string Buffer::getType<bool>    (const bool &)     { return {'\x01'}; }
	template <> std::string Buffer::getType<uint8_t> (const uint8_t &)  { return {'\x01'}; }
	template <> std::string Buffer::getType<uint16_t>(const uint16_t &) { return {'\x02'}; }
	template <> std::string Buffer::getType<uint32_t>(const uint32_t &) { return {'\x03'}; }
	template <> std::string Buffer::getType<uint64_t>(const uint64_t &) { return {'\x04'}; }
	template <> std::string Buffer::getType<char>    (const char &)     { return {'\x05'}; }
	template <> std::string Buffer::getType<int8_t>  (const int8_t &)   { return {'\x05'}; }
	template <> std::string Buffer::getType<int16_t> (const int16_t &)  { return {'\x06'}; }
	template <> std::string Buffer::getType<int32_t> (const int32_t &)  { return {'\x07'}; }
	template <> std::string Buffer::getType<int64_t> (const int64_t &)  { return {'\x08'}; }
	template <> std::string Buffer::getType<float>   (const float &)    { return {'\x09'}; }
	template <> std::string Buffer::getType<double>  (const double &)   { return {'\x0a'}; }

	template <> Buffer & Buffer::appendType<bool>    (const bool     &) { return *this += '\x01'; }
	template <> Buffer & Buffer::appendType<uint8_t> (const uint8_t  &) { return *this += '\x01'; }
	template <> Buffer & Buffer::appendType<uint16_t>(const uint16_t &) { return *this += '\x02'; }
	template <> Buffer & Buffer::appendType<uint32_t>(const uint32_t &) { return *this += '\x03'; }
	template <> Buffer & Buffer::appendType<uint64_t>(const uint64_t &) { return *this += '\x04'; }
	template <> Buffer & Buffer::appendType<char>    (const char     &) { return *this += '\x05'; }
	template <> Buffer & Buffer::appendType<int8_t>  (const int8_t   &) { return *this += '\x05'; }
	template <> Buffer & Buffer::appendType<int16_t> (const int16_t  &) { return *this += '\x06'; }
	template <> Buffer & Buffer::appendType<int32_t> (const int32_t  &) { return *this += '\x07'; }
	template <> Buffer & Buffer::appendType<int64_t> (const int64_t  &) { return *this += '\x08'; }
	template <> Buffer & Buffer::appendType<float>   (const float    &) { return *this += '\x09'; }
	template <> Buffer & Buffer::appendType<double>  (const double   &) { return *this += '\x0a'; }

	template <>
	std::string Buffer::getType<std::string_view>(const std::string_view &string) {
		const auto size = string.size();

		if (size == 0)
			return {'\x10'};

		if (size < 0xf)
			return {static_cast<char>('\x10' + size)};

		assert(size <= UINT32_MAX);
		return {'\x1f'};
	}

	template <>
	std::string Buffer::getType<std::string>(const std::string &string) {
		return getType(std::string_view(string));
	}

	Buffer & Buffer::operator+=(bool item) {
		bytes.insert(bytes.end(), static_cast<uint8_t>(item));
		return *this;
	}

	Buffer & Buffer::operator+=(char item) {
		bytes.insert(bytes.end(), static_cast<uint8_t>(item));
		return *this;
	}

	Buffer & Buffer::operator+=(uint8_t item) {
		bytes.insert(bytes.end(), item);
		return *this;
	}

	Buffer & Buffer::operator+=(uint16_t item) {
		if constexpr (std::endian::native == std::endian::little) {
			const auto *raw = reinterpret_cast<uint8_t *>(&item);
			bytes.insert(bytes.end(), raw, raw + sizeof(item));
		} else
			bytes.insert(bytes.end(), {static_cast<uint8_t>(item), static_cast<uint8_t>(item >> 8)});
		return *this;
	}

	Buffer & Buffer::operator+=(uint32_t item) {
		if constexpr (std::endian::native == std::endian::little) {
			const auto *raw = reinterpret_cast<uint8_t *>(&item);
			bytes.insert(bytes.end(), raw, raw + sizeof(item));
		} else
			bytes.insert(bytes.end(), {static_cast<uint8_t>(item), static_cast<uint8_t>(item >> 8), static_cast<uint8_t>(item >> 16), static_cast<uint8_t>(item >> 24)});
		return *this;
	}

	Buffer & Buffer::operator+=(uint64_t item) {
		if constexpr (std::endian::native == std::endian::little) {
			const auto *raw = reinterpret_cast<uint8_t *>(&item);
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

	Buffer & Buffer::operator+=(float item) {
		static_assert(sizeof(item) == 4 && sizeof(item) == sizeof(uint32_t));
		uint32_t to_add{};
		std::memcpy(&to_add, &item, sizeof(to_add));
		return *this += to_add;
	}

	Buffer & Buffer::operator+=(double item) {
		static_assert(sizeof(item) == 8 && sizeof(item) == sizeof(uint64_t));
		uint64_t to_add{};
		std::memcpy(&to_add, &item, sizeof(to_add));
		return *this += to_add;
	}

	Buffer & Buffer::operator+=(std::string_view string) {
		const auto type = getType(string);
		bytes.insert(bytes.end(), type.begin(), type.end());
		const auto first = type[0];

		if (first == '\x10')
			return *this;

		if ('\x11' <= first && first < '\x1f') {
			bytes.insert(bytes.end(), string.begin(), string.end());
			return *this;
		}

		assert(string.size() <= UINT32_MAX);
		*this += static_cast<uint32_t>(string.size());
		bytes.insert(bytes.end(), string.begin(), string.end());
		return *this;
	}

	template <>
	char popBuffer<char>(Buffer &buffer) {
		std::span span = buffer.getSpan();
		if (span.empty()) {
			ERROR("Buffer size: " << buffer.bytes.size());
			ERROR("Skip: " << buffer.skip);
			ERROR("Span size: " << span.size());
			ERROR("Span size_bytes: " << span.size_bytes());
			INFO(hexString(buffer.bytes, true));
			throw std::out_of_range("Buffer is empty");
		}
		const char out = span[0];
		++buffer.skip;
		return out;
	}

	template <>
	bool popBuffer<bool>(Buffer &buffer) {
		return static_cast<bool>(popBuffer<char>(buffer));
	}

	template <>
	float popBuffer<float>(Buffer &buffer) {
		static_assert(sizeof(float) == sizeof(uint32_t));
		const auto raw = popBuffer<uint32_t>(buffer);
		float out{};
		std::memcpy(&out, &raw, sizeof(out));
		return out;
	}

	template <>
	double popBuffer<double>(Buffer &buffer) {
		static_assert(sizeof(double) == sizeof(uint64_t));
		const auto raw = popBuffer<uint64_t>(buffer);
		double out{};
		std::memcpy(&out, &raw, sizeof(out));
		return out;
	}

	template <>
	std::string popBuffer<std::string>(Buffer &buffer) {
		return buffer.take<std::string>();
	}

	std::string Buffer::popType() {
		const char first = popBuffer<char>(*this);
		if (('\x01' <= first && first <= '\x0c') || ('\x10' <= first && first <= '\x1f') || ('\xe0' <= first && first <= '\xe2'))
			return {first};
		if (first == '\x20')
			return first + popType();
		if (first == '\x21') {
			std::string key_type = popType();
			std::string value_type = popType();
			return first + std::move(key_type) + std::move(value_type);
		}
		debug();
		throw std::invalid_argument("Invalid type byte: " + hexString(std::string_view(&first, 1), true));
	}

	bool Buffer::typesMatch(std::string_view one, std::string_view two) {
		assert(!one.empty());
		assert(!two.empty());
		const auto one0 = one[0];
		const auto two0 = two[0];
		if (('\x10' <= one0 && one0 <= '\x1f') && ('\x10' <= two0 && two0 <= '\x1f'))
			return true;
		if ((one0 == '\x0b' && two0 == '\x0c') || (one0 == '\x0c' && two0 == '\x0b'))
			return true;
		return one == two;
	}

	void Buffer::popMany(size_t count) {
		assert(count <= size() - skip);
		skip += count;
	}

	void Buffer::limitTo(size_t count) {
		if (count < bytes.size() - skip)
			skip += count;
	}

	void Buffer::debug() const {
		if (skip == 0)
			INFO("Buffer: " << hexString(bytes, true));
		else
			INFO("Buffer: \e[2m" << hexString(std::span(bytes.begin(), bytes.begin() + skip), true) << "\e[22m " << hexString(std::span(bytes.begin() + skip, bytes.end()), true));
	}

	Buffer & Buffer::operator<<(bool item) {
		return appendType(item) += item;
	}

	Buffer & Buffer::operator<<(uint8_t item) {
		return appendType(item) += item;
	}

	Buffer & Buffer::operator<<(uint16_t item) {
		return appendType(item) += item;
	}

	Buffer & Buffer::operator<<(uint32_t item) {
		return appendType(item) += item;
	}

	Buffer & Buffer::operator<<(uint64_t item) {
		return appendType(item) += item;
	}

	Buffer & Buffer::operator<<(int8_t item) {
		return appendType(item) += static_cast<uint8_t>(item);
	}

	Buffer & Buffer::operator<<(int16_t item) {
		return appendType(item) += static_cast<uint16_t>(item);
	}

	Buffer & Buffer::operator<<(int32_t item) {
		return appendType(item) += static_cast<uint32_t>(item);
	}

	Buffer & Buffer::operator<<(int64_t item) {
		return appendType(item) += static_cast<uint64_t>(item);
	}

	Buffer & Buffer::operator<<(float item) {
		return appendType(item) += item;
	}

	Buffer & Buffer::operator<<(double item) {
		return appendType(item) += item;
	}

	Buffer & Buffer::operator<<(std::string_view string) {
		return *this += string;
	}

	Buffer & Buffer::operator<<(const std::string &string) {
		return *this << std::string_view(string);
	}

	Buffer & Buffer::operator<<(const char *string) {
		return *this << std::string_view(string);
	}

	Buffer & Buffer::operator<<(const Buffer &other) {
		append(other.bytes.begin(), other.bytes.end());
		return *this;
	}

	std::ostream & operator<<(std::ostream &os, const Buffer &buffer) {
		os << "Buffer<";

		for (bool first = true; const uint16_t byte: buffer.bytes) {
			if (first)
				first = false;
			else
				os << ' ';
			os << std::hex << std::setw(2) << std::setfill('0') << std::right << byte << std::dec;
		}

		return os << ">[" << buffer.size() << ']';
	}

	template <>
	Buffer & Buffer::operator>><std::string>(std::string &out) {
		const std::string type = popType();
		const char front = type.front();
		uint32_t size{};
		if (front == '\x1f') {
			size = popBuffer<uint32_t>(*this);
		} else if ('\x10' <= front && front < '\x1f') {
			size = front - '\x10';
		} else {
			debug();
			throw std::invalid_argument("Invalid type in buffer (expected string): " + hexString(std::string_view(&front, 1), true));
		}
		out.clear();
		out.reserve(size);
		for (uint32_t i = 0; i < size; ++i)
			out.push_back(popBuffer<char>(*this));
		return *this;
	}

	template <>
	Buffer & Buffer::operator>>(Buffer &other) {
		const std::span<const uint8_t> span = getSpan();
		other.append(span.begin(), span.end());
		bytes.clear();
		skip = 0;
		return *this;
	}
}
