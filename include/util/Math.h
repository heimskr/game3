#pragma once

#include <bit>
#include <cmath>
#include <type_traits>

namespace Game3 {
	template <typename T>
	inline T updiv(T n, std::type_identity_t<T> d) {
		return n / d + (n % d? 1 : 0);
	}

	inline double fractional(double d) {
		return std::modf(d, &d);
	}

	inline char swapBytes(char in) {
		return in;
	}

	inline uint8_t swapBytes(uint8_t in) {
		return in;
	}

	inline uint16_t swapBytes(uint16_t in) {
		return ((in >> 8) & 0xff) | ((in & 0xff) << 8);
	}

	inline uint32_t swapBytes(uint32_t in) {
		return ((in >> 24) & 0xff) | (((in >> 16) & 0xff) << 8) | (((in >> 8) & 0xff) << 16) | ((in & 0xff) << 24);
	}

	inline uint64_t swapBytes(uint64_t in) {
		return  ((in >> 56) & 0xff)
		     | (((in >> 48) & 0xff) <<  8)
		     | (((in >> 40) & 0xff) << 16)
		     | (((in >> 32) & 0xff) << 24)
		     | (((in >> 24) & 0xff) << 32)
		     | (((in >> 16) & 0xff) << 40)
		     | (((in >>  8) & 0xff) << 48)
		     |  ((in        & 0xff) << 56);
	}

	template <typename T>
	inline T toLittle(T in) {
		if constexpr (std::endian::native == std::endian::little)
			return in;
		return swapBytes(in);
	}
}
