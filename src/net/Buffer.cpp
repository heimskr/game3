#include "Log.h"
#include "biology/Gene.h"
#include "game/Fluids.h"
#include "game/ServerInventory.h"
#include "item/Item.h"
#include "lib/JSON.h"
#include "net/Buffer.h"
#include "pipes/ItemFilter.h"
#include "types/Position.h"
#include "util/Util.h"

#include <cassert>
#include <cstring>
#include <iomanip>

namespace Game3 {
	Buffer::Buffer(Side target):
		target(target) {}

	Buffer::Buffer(std::vector<uint8_t> bytes, std::weak_ptr<BufferContext> context, Side target):
		bytes(std::move(bytes)), target(target), context(std::move(context)) {}

	Buffer::Buffer(std::weak_ptr<BufferContext> context, Side target):
		target(target), context(std::move(context)) {}

	Buffer::Buffer(std::vector<uint8_t> bytes, Side target):
		bytes(std::move(bytes)), target(target) {}

	Buffer::Buffer(Buffer &&other) noexcept:
		bytes(std::move(other.bytes)),
		skip(std::exchange(other.skip, 0)),
		target(other.target),
		context(std::move(other.context)) {}

	Buffer & Buffer::operator=(Buffer &&other) noexcept {
		bytes = std::move(other.bytes);
		skip = std::exchange(other.skip, 0);
		context = std::move(other.context);
		target = other.target;
		return *this;
	}

	template <> std::string Buffer::getType<bool>    (const bool     &, bool) { return {'\x01'}; }
	template <> std::string Buffer::getType<uint8_t> (const uint8_t  &, bool) { return {'\x01'}; }
	template <> std::string Buffer::getType<uint16_t>(const uint16_t &, bool) { return {'\x02'}; }
	template <> std::string Buffer::getType<uint32_t>(const uint32_t &, bool) { return {'\x03'}; }
	template <> std::string Buffer::getType<uint64_t>(const uint64_t &, bool) { return {'\x04'}; }
	template <> std::string Buffer::getType<char>    (const char     &, bool) { return {'\x05'}; }
	template <> std::string Buffer::getType<int8_t>  (const int8_t   &, bool) { return {'\x05'}; }
	template <> std::string Buffer::getType<int16_t> (const int16_t  &, bool) { return {'\x06'}; }
	template <> std::string Buffer::getType<int32_t> (const int32_t  &, bool) { return {'\x07'}; }
	template <> std::string Buffer::getType<int64_t> (const int64_t  &, bool) { return {'\x08'}; }
	template <> std::string Buffer::getType<float>   (const float    &, bool) { return {'\x09'}; }
	template <> std::string Buffer::getType<double>  (const double   &, bool) { return {'\x0a'}; }
	template <> std::string Buffer::getType<std::nullopt_t>(const std::nullopt_t &, bool in_container) { assert(!in_container); return {'\x0c'}; }

	template <> Buffer & Buffer::appendType<bool>    (const bool     &, bool) { return *this += '\x01'; }
	template <> Buffer & Buffer::appendType<uint8_t> (const uint8_t  &, bool) { return *this += '\x01'; }
	template <> Buffer & Buffer::appendType<uint16_t>(const uint16_t &, bool) { return *this += '\x02'; }
	template <> Buffer & Buffer::appendType<uint32_t>(const uint32_t &, bool) { return *this += '\x03'; }
	template <> Buffer & Buffer::appendType<uint64_t>(const uint64_t &, bool) { return *this += '\x04'; }
	template <> Buffer & Buffer::appendType<char>    (const char     &, bool) { return *this += '\x05'; }
	template <> Buffer & Buffer::appendType<int8_t>  (const int8_t   &, bool) { return *this += '\x05'; }
	template <> Buffer & Buffer::appendType<int16_t> (const int16_t  &, bool) { return *this += '\x06'; }
	template <> Buffer & Buffer::appendType<int32_t> (const int32_t  &, bool) { return *this += '\x07'; }
	template <> Buffer & Buffer::appendType<int64_t> (const int64_t  &, bool) { return *this += '\x08'; }
	template <> Buffer & Buffer::appendType<float>   (const float    &, bool) { return *this += '\x09'; }
	template <> Buffer & Buffer::appendType<double>  (const double   &, bool) { return *this += '\x0a'; }
	template <> Buffer & Buffer::appendType<std::nullopt_t>(const std::nullopt_t &, bool in_container) { assert(!in_container); return *this += '\x0c'; }

	template <>
	std::string Buffer::getType<std::string_view>(const std::string_view &string, bool in_container) {
		if (!in_container) {
			const auto size = string.size();

			if (size == 0)
				return {'\x10'};

			if (size < 0xf)
				return {static_cast<char>('\x10' + size)};

			assert(size <= UINT32_MAX);
		}

		return {'\x1f'};
	}

	template <>
	std::string Buffer::getType<std::string>(const std::string &string, bool in_container) {
		return getType(std::string_view(string), in_container);
	}

	template <>
	std::string Buffer::getType<boost::json::value>(const boost::json::value &, bool in_container) {
		return getType(std::string{}, in_container);
	}

	template<>
	Buffer & Buffer::operator+=(bool item) {
		bytes.insert(bytes.end(), static_cast<uint8_t>(item));
		return *this;
	}

	template<>
	Buffer & Buffer::operator+=(uint8_t item) {
		bytes.insert(bytes.end(), item);
		return *this;
	}

	template<>
	Buffer & Buffer::operator+=(uint16_t item) {
		if constexpr (std::endian::native == std::endian::little) {
			const auto *raw = reinterpret_cast<uint8_t *>(&item);
			bytes.insert(bytes.end(), raw, raw + sizeof(item));
		} else
			bytes.insert(bytes.end(), {static_cast<uint8_t>(item), static_cast<uint8_t>(item >> 8)});
		return *this;
	}

	template<>
	Buffer & Buffer::operator+=(uint32_t item) {
		if constexpr (std::endian::native == std::endian::little) {
			const auto *raw = reinterpret_cast<uint8_t *>(&item);
			bytes.insert(bytes.end(), raw, raw + sizeof(item));
		} else
			bytes.insert(bytes.end(), {static_cast<uint8_t>(item), static_cast<uint8_t>(item >> 8), static_cast<uint8_t>(item >> 16), static_cast<uint8_t>(item >> 24)});
		return *this;
	}

	template<>
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

	template<>
	Buffer & Buffer::operator+=(float item) {
		static_assert(sizeof(item) == 4 && sizeof(item) == sizeof(uint32_t));
		uint32_t to_add{};
		std::memcpy(&to_add, &item, sizeof(to_add));
		return *this += to_add;
	}

	template<>
	Buffer & Buffer::operator+=(double item) {
		static_assert(sizeof(item) == 8 && sizeof(item) == sizeof(uint64_t));
		uint64_t to_add{};
		std::memcpy(&to_add, &item, sizeof(to_add));
		return *this += to_add;
	}

	template<>
	Buffer & Buffer::operator+=(std::string_view string) {
		const auto type = getType(string, false);
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

	template<>
	Buffer & Buffer::operator+=(const std::string &string) {
		return *this += std::string_view(string);
	}

	template<>
	Buffer & Buffer::operator+=(const boost::json::value &json) {
		return *this += boost::json::serialize(json);
	}

	template <>
	char popBuffer<char>(Buffer &buffer) {
		std::span span = buffer.getSpan();
		if (span.empty()) {
			ERROR("Buffer size: {:L}", buffer.bytes.size());
			ERROR("Skip: {:L}", buffer.skip);
			ERROR("Span size: {:L}", span.size());
			ERROR("Span size_bytes: {:L}", span.size_bytes());
			INFO("{}", hexString(buffer.bytes, true));
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

	template <>
	boost::json::value popBuffer<boost::json::value>(Buffer &buffer) {
		return buffer.take<boost::json::value>();
	}

	std::string Buffer::popType() {
		const char first = popBuffer<char>(*this);
		if (('\x01' <= first && first <= '\x0c') || ('\x10' <= first && first <= '\x1f') || ('\xe0' <= first && first <= '\xe9'))
			return {first};
		if (first == '\x20' || ('\x30' <= first && first <= '\x3f'))
			return first + popType();
		if (first == '\x21') {
			std::string key_type = popType();
			std::string value_type = popType();
			return first + std::move(key_type) + std::move(value_type);
		}
		debug();
		throw std::invalid_argument("Invalid type byte: " + hexString(std::string_view(&first, 1), true));
	}

	std::string Buffer::peekType(size_t to_skip) {
		const char first = peek<char>(to_skip);
		if (('\x01' <= first && first <= '\x0c') || ('\x10' <= first && first <= '\x1f') || ('\xe0' <= first && first <= '\xe9'))
			return {first};
		if (first == '\x20' || ('\x30' <= first && first <= '\x3f'))
			return first + peekType(to_skip + 1);
		if (first == '\x21') {
			std::string key_type = peekType(to_skip + 1);
			std::string value_type = peekType(to_skip + 1 + key_type.size());
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
			INFO("Buffer: {}", hexString(bytes, true));
		else
			INFO("Buffer: \e[2m{}\e[22m {}", hexString(std::span(bytes.begin(), bytes.begin() + skip), true), hexString(std::span(bytes.begin() + skip, bytes.end()), true));
	}

	size_t Buffer::getSkip() const {
		return skip;
	}

	void Buffer::setSkip(size_t new_skip) {
		if (new_skip > bytes.size()) {
			throw std::out_of_range(std::format("New skip value {} too high (must not exceed {})", new_skip, bytes.size()));
		}

		skip = new_skip;
	}

	namespace {
		std::string stringifyKey(const boost::json::value &json) {
			if (const auto *value = json.if_string()) {
				return std::string(*value);
			}

			if (const double *value = json.if_double()) {
				return std::to_string(*value);
			}

			if (const int64_t *value = json.if_int64()) {
				return std::to_string(*value);
			}

			if (const uint64_t *value = json.if_uint64()) {
				return std::to_string(*value);
			}

			if (json.is_null()) {
				return "null";
			}

			if (const bool *value = json.if_bool()) {
				return *value? "true" : "false";
			}

			return boost::json::serialize(json);
		}

		std::string_view extractType(std::string_view type);

		std::pair<std::string_view, std::string_view> extractMapTypes(std::string_view type) {
			assert(!type.empty());

			assert(type[0] == '\x21');
			type.remove_prefix(1);

			std::string_view key, value;

			key = extractType(type);
			type.remove_prefix(key.size());

			value = extractType(type);

			return {key, value};
		}

		std::string_view extractType(std::string_view type) {
			assert(!type.empty());

			char first = type[0];
			if (('\x01' <= first && first <= '\x0c') || ('\x10' <= first && first <= '\x1f') || ('\xe0' <= first && first <= '\xe9'))
				return type.substr(0, 1);

			if (first == '\x20' || ('\x30' <= first && first <= '\x3f'))
				return type.substr(0, 1 + extractType(type.substr(1)).size());

			if (first == '\x21') {
				auto [key, value] = extractMapTypes(type);
				return type.substr(0, 1 + key.size() + value.size());
			}

			throw std::invalid_argument("Invalid type byte: " + hexString(type.substr(0, 1), true));
		}

		boost::json::value popJSON(Buffer &buffer, std::string_view type, bool in_container) {
			assert(!type.empty());

			if (!in_container) {
				switch (type[0]) {
					case '\x01': return buffer.take<uint8_t>();
					case '\x02': return buffer.take<uint16_t>();
					case '\x03': return buffer.take<uint32_t>();
					case '\x04': return buffer.take<uint64_t>();
					case '\x05': return buffer.take<int8_t>();
					case '\x06': return buffer.take<int16_t>();
					case '\x07': return buffer.take<int32_t>();
					case '\x08': return buffer.take<int64_t>();
					case '\x09': return buffer.take<float>();
					case '\x0a': return buffer.take<double>();

					case '\x0b': ++buffer.skip; return buffer.popJSON();

					case '\x0c': return nullptr;

					case '\x10': case '\x11': case '\x12': case '\x13': case '\x14': case '\x15': case '\x16': case '\x17':
					case '\x18': case '\x19': case '\x1a': case '\x1b': case '\x1c': case '\x1d': case '\x1e': case '\x1f':
						return boost::json::value_from(buffer.take<std::string>());

					case '\x20': {
						++buffer.skip;
						std::string type = buffer.peekType(0);
						buffer.skip += type.size();
						const uint32_t length = popBuffer<uint32_t>(buffer);
						boost::json::value out;
						auto &array = out.emplace_array();
						for (uint32_t i = 0; i < length; ++i) {
							array.push_back(popJSON(buffer, type, true));
						}
						return out;
					}

					case '\x21': {
						++buffer.skip;
						std::string key_type = buffer.peekType(0);
						buffer.skip += key_type.size();
						std::string value_type = buffer.peekType(0);
						buffer.skip += value_type.size();
						const uint32_t length = popBuffer<uint32_t>(buffer);
						boost::json::value out;
						auto &object = out.emplace_object();
						for (uint32_t i = 0; i < length; ++i) {
							boost::json::value key = popJSON(buffer, key_type, true);
							boost::json::value value = popJSON(buffer, value_type, true);
							if (const auto *string = key.if_string()) {
								object[*string] = std::move(value);
							} else {
								object[stringifyKey(key)] = std::move(value);
							}
						}
						return out;
					}

					case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37':
					case '\x38': case '\x39': case '\x3a': case '\x3b': case '\x3c': case '\x3d': case '\x3e': case '\x3f': {
						const uint32_t length(type[0] - '\x30');
						++buffer.skip;
						std::string type = buffer.peekType(0);
						buffer.skip += type.size();
						boost::json::value out;
						auto &array = out.emplace_array();
						for (uint32_t i = 0; i < length; ++i) {
							array.push_back(popJSON(buffer, type, true));
						}
						return out;
					}
				}
			}

			switch (type[0]) {
				case '\x01': return popBuffer<uint8_t>(buffer);
				case '\x02': return popBuffer<uint16_t>(buffer);
				case '\x03': return popBuffer<uint32_t>(buffer);
				case '\x04': return popBuffer<uint64_t>(buffer);
				case '\x05': return popBuffer<int8_t>(buffer);
				case '\x06': return popBuffer<int16_t>(buffer);
				case '\x07': return popBuffer<int32_t>(buffer);
				case '\x08': return popBuffer<int64_t>(buffer);
				case '\x09': return popBuffer<float>(buffer);
				case '\x0a': return popBuffer<double>(buffer);

				case '\x0b': return popJSON(buffer, type.substr(1), true);

				case '\x0c': assert(!"Empty optional in container subtype"); return {};

				case '\x10': case '\x11': case '\x12': case '\x13': case '\x14': case '\x15': case '\x16': case '\x17':
				case '\x18': case '\x19': case '\x1a': case '\x1b': case '\x1c': case '\x1d': case '\x1e':
					assert(!"Short string in container subtype");
					return {};

				case '\x1f':
					return boost::json::value_from(popBuffer<std::string>(buffer));

				case '\x20':
				case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37':
				case '\x38': case '\x39': case '\x3a': case '\x3b': case '\x3c': case '\x3d': case '\x3e': case '\x3f': {
					const uint32_t length = type[0] == '\x20'? popBuffer<uint32_t>(buffer) : type[0] - '\x30';
					boost::json::value out;
					auto &array = out.emplace_array();
					std::string_view subtype = type.substr(1);
					for (uint32_t i = 0; i < length; ++i) {
						array.push_back(popJSON(buffer, subtype, true));
					}
					return out;
				}

				case '\x21': {
					auto [key_type, value_type] = extractMapTypes(type);
					buffer.skip += type.size();
					const uint32_t length = popBuffer<uint32_t>(buffer);
					boost::json::value out;
					auto &object = out.emplace_object();
					for (uint32_t i = 0; i < length; ++i) {
						boost::json::value key = popJSON(buffer, key_type, true);
						boost::json::value value = popJSON(buffer, value_type, true);
						if (const auto *string = key.if_string()) {
							object[*string] = std::move(value);
						} else {
							object[stringifyKey(key)] = std::move(value);
						}
					}
					return out;
				}

				case '\xe0':
					return boost::json::value_from(*buffer.take<ItemStackPtr>());

				case '\xe1':
					return boost::json::value_from(buffer.take<ServerInventory>());

				case '\xe2':
					return boost::json::value_from(std::format("FluidStack<{}>", std::string(buffer.take<FluidStack>())));

				case '\xe3': {
					buffer.take<ItemFilter>();
					return "<ItemFilter>";
				}

				case '\xe4': {
					buffer.take<ItemFilter::Config>();
					return "<ItemFilter::Config>";
				}

				case '\xe5':
					return boost::json::value_from(buffer.take<FloatGene>());

				case '\xe6':
					return boost::json::value_from(buffer.take<LongGene>());

				case '\xe7':
					return boost::json::value_from(buffer.take<CircularGene>());

				case '\xe8':
					return boost::json::value_from(buffer.take<StringGene>());

				case '\xe9':
					return boost::json::value_from(buffer.take<Position>());
			}

			return {};
		}
	}

	boost::json::value Buffer::popJSON() {
		return Game3::popJSON(*this, peekType(0), false);
	}

	boost::json::value Buffer::popAllJSON() {
		boost::json::value out;
		auto &array = out.emplace_array();
		while (!empty()) {
			array.push_back(popJSON());
		}
		return out;
	}

	template <>
	bool Buffer::peek<bool>(size_t to_skip) const {
		return static_cast<bool>(peek<char>(to_skip));
	}

	template <>
	float Buffer::peek<float>(size_t to_skip) const {
		static_assert(sizeof(float) == sizeof(uint32_t));
		const auto raw = peek<uint32_t>(to_skip);
		float out{};
		std::memcpy(&out, &raw, sizeof(out));
		return out;
	}

	template <>
	double Buffer::peek<double>(size_t to_skip) const {
		static_assert(sizeof(double) == sizeof(uint64_t));
		const auto raw = peek<uint64_t>(to_skip);
		double out{};
		std::memcpy(&out, &raw, sizeof(out));
		return out;
	}

	Buffer & Buffer::operator<<(bool item) {
		return appendType(item, false) += item;
	}

	Buffer & Buffer::operator<<(uint8_t item) {
		return appendType(item, false) += item;
	}

	Buffer & Buffer::operator<<(uint16_t item) {
		return appendType(item, false) += item;
	}

	Buffer & Buffer::operator<<(uint32_t item) {
		return appendType(item, false) += item;
	}

	Buffer & Buffer::operator<<(uint64_t item) {
		return appendType(item, false) += item;
	}

	Buffer & Buffer::operator<<(int8_t item) {
		return appendType(item, false) += static_cast<uint8_t>(item);
	}

	Buffer & Buffer::operator<<(int16_t item) {
		return appendType(item, false) += static_cast<uint16_t>(item);
	}

	Buffer & Buffer::operator<<(int32_t item) {
		return appendType(item, false) += static_cast<uint32_t>(item);
	}

	Buffer & Buffer::operator<<(int64_t item) {
		return appendType(item, false) += static_cast<uint64_t>(item);
	}

	Buffer & Buffer::operator<<(float item) {
		return appendType(item, false) += item;
	}

	Buffer & Buffer::operator<<(double item) {
		return appendType(item, false) += item;
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

	template <>
	Buffer & operator>>(Buffer &buffer, std::string &out) {
		const std::string type = buffer.popType();
		const char front = type.front();
		uint32_t size{};
		if (front == '\x1f') {
			size = popBuffer<uint32_t>(buffer);
		} else if ('\x10' <= front && front < '\x1f') {
			size = front - '\x10';
		} else {
			buffer.debug();
			throw std::invalid_argument("Invalid type in buffer (expected string): " + hexString(std::string_view(&front, 1), true));
		}
		out.clear();
		out.reserve(size);
		for (uint32_t i = 0; i < size; ++i)
			out.push_back(popBuffer<char>(buffer));
		return buffer;
	}

	template <>
	Buffer & operator>>(Buffer &buffer, Buffer &other) {
		const std::span<const uint8_t> span = buffer.getSpan();
		other.append(span.begin(), span.end());
		buffer.bytes.clear();
		buffer.skip = 0;
		return buffer;
	}

	template<>
	Buffer & operator<<(Buffer &buffer, const boost::json::value &json) {
		return buffer << boost::json::serialize(json);
	}

	template<>
	Buffer & operator>>(Buffer &buffer, boost::json::value &json) {
		json = boost::json::parse(buffer.take<std::string>());
		return buffer;
	}
}
