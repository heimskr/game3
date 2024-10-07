#pragma once

#include <bit>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <type_traits>

namespace Game3 {
	template <std::floating_point F, typename R>
	inline F replaceNaN(F number, R replacement) {
		return std::isnan(number)? static_cast<F>(replacement) : number;
	}

	inline auto updiv(auto n, auto d) {
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

	constexpr float lerp(float from, float to, float progress) {
		return from + (to - from) * progress;
	}

	template <std::integral I>
	I sqrt(I n) {
		// Credit: https://stackoverflow.com/a/63457507
		uint8_t shift(std::bit_width(n));

		shift += shift & 1; // round up to next multiple of 2

		I result = 0;

		do {
			shift -= 2;
			result <<= 1; // make space for the next guessed bit
			result |= 1;  // guess that the next bit is 1
			result ^= result * result > (n >> shift); // revert if guess too high
		} while (shift != 0);

		return result;
	}
}
