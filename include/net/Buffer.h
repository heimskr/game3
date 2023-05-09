#pragma once

#include <bit>
#include <concepts>
#include <cstdint>
#include <deque>
#include <iostream>
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
			Buffer & appendType(const T &t) {
				const auto type = getType(t);
				bytes.insert(bytes.end(), type.begin(), type.end());
				return *this;
			}

			template <typename T>
			std::string getType(const T &);

			// Inelegant how it requires an instance of the type as an argument.
			template <Map M>
			std::string getType(const M &) {
				return '\x21' + getType(typename M::key_type()) + getType(typename M::mapped_type());
			}

			// Same here.
			template <LinearContainer T>
			std::string getType(const T &) {
				return '\x20' + getType(typename T::value_type());
			}

			Buffer & append(char);
			Buffer & append(uint8_t);
			Buffer & append(uint16_t);
			Buffer & append(uint32_t);
			Buffer & append(uint64_t);
			Buffer & append(std::string_view);

			template <Map M>
			Buffer & append(const M &map) {
				assert(map.size() <= UINT32_MAX);
				append(static_cast<uint32_t>(map.size()));
				for (const auto &[key, value]: map)
					append(key).append(value);
				return *this;
			}

			template <LinearContainer T>
			Buffer & append(const T &container) {
				assert(container.size() <= UINT32_MAX);
				append(static_cast<uint32_t>(container.size()));
				for (const auto &item: container)
					append(item);
				return *this;
			}

			std::string popType();

			template <typename T1, typename T2>
			inline T2 popConv() {
				return static_cast<T2>(popRaw<T1>());
			}

			static bool typesMatch(std::string_view, std::string_view);

		public:
			Buffer() = default;

			inline auto size() const { return bytes.size(); }

			Buffer & operator<<(uint8_t);
			Buffer & operator<<(uint16_t);
			Buffer & operator<<(uint32_t);
			Buffer & operator<<(uint64_t);
			Buffer & operator<<(int8_t);
			Buffer & operator<<(int16_t);
			Buffer & operator<<(int32_t);
			Buffer & operator<<(int64_t);
			Buffer & operator<<(std::string_view);
			Buffer & operator<<(const std::string &);

			template <Map M>
			Buffer & operator<<(const M &map) {
				return appendType(map).append(map);
			}

			template <LinearContainer T>
			Buffer & operator<<(const T &container) {
				return appendType(container).append(container);
			}

			friend std::ostream & operator<<(std::ostream &, const Buffer &);

			template <typename T>
			T popRaw();

			template <LinearContainer C>
			C popRaw() {
				const auto size = popRaw<uint32_t>();
				C out;
				if constexpr (Reservable<C>)
					out.reserve(size);

				for (uint32_t i = 0; i < size; ++i)
					out.push_back(popRaw<typename C::value_type>());

				return out;
			}

			template <Map M>
			M popRaw() {
				const auto size = popRaw<uint32_t>();
				M out;
				for (uint32_t i = 0; i < size; ++i)
					out.emplace(popRaw<typename M::key_type>(), popRaw<typename M::mapped_type>());
				return out;
			}

			template <typename T>
			T pop();

			template <LinearContainer C>
			C pop() {
				if (!typesMatch(popType(), getType(C())))
					throw std::invalid_argument("Invalid type in buffer");
				return popRaw<C>();
			}

			template <Map M>
			M pop() {
				if (!typesMatch(popType(), getType(M())))
					throw std::invalid_argument("Invalid type in buffer");
				return popRaw<M>();
			}

			template <std::integral T>
			T pop() {
				if (!typesMatch(popType(), getType(T())))
					throw std::invalid_argument("Invalid type in buffer");
				return popRaw<T>();
			}
	};

	std::ostream & operator<<(std::ostream &, const Buffer &);
}
