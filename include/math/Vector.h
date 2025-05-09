#pragma once

#include "math/Concepts.h"

#include <cmath>
#include <format>

namespace Game3 {
	class Buffer;
	struct Position;

	struct Vector3 {
		double x = 0;
		double y = 0;
		double z = 0;

		double magnitude() const {
			return std::sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2));
		}

		double magnitude2D() const {
			return std::sqrt(std::pow(x, 2) + std::pow(y, 2));
		}

		bool isGrounded() const {
			return z < 0.01;
		}

		bool operator==(const Vector3 &other) const {
			return this == &other || (x == other.x && y == other.y && z == other.z);
		}

		explicit operator bool() const {
			return x != 0 || y != 0 || z != 0;
		}

		Vector3 & operator+=(const Vector3 &other) {
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		Vector3 & operator-=(const Vector3 &other) {
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}

		Vector3 & operator*=(const Vector3 &other) {
			x *= other.x;
			y *= other.y;
			z *= other.z;
			return *this;
		}

		Vector3 operator+(const Vector3 &other) const {
			return {x + other.x, y + other.y, z + other.z};
		}

		Vector3 operator-(const Vector3 &other) const {
			return {x - other.x, y - other.y, z - other.z};
		}

		Vector3 operator*(const Vector3 &other) const {
			return {x * other.x, y * other.y, z * other.z};
		}

		template <Numeric N>
		Vector3 operator*(N multiplier) {
			return {x * multiplier, y * multiplier, z * multiplier};
		}

		template <Numeric N>
		Vector3 & operator*=(N multiplier) {
			x *= multiplier;
			y *= multiplier;
			z *= multiplier;
			return *this;
		}
	};

	Buffer & operator+=(Buffer &, const Vector3 &);
	Buffer & operator<<(Buffer &, const Vector3 &);
	Buffer & operator>>(Buffer &, Vector3 &);

	struct Vector2d {
		double x = 0;
		double y = 0;

		Vector2d() = default;

		Vector2d(double x, double y):
			x(x),
			y(y) {}

		Vector2d(Position);

		double magnitude() const {
			return std::sqrt(x * x + y * y);
		}

		double distance(const Vector2d &other) const {
			return std::sqrt(std::pow(other.x - x, 2) + std::pow(other.y - y, 2));
		}

		bool operator==(const Vector2d &other) const {
			return this == &other || (x == other.x && y == other.y);
		}

		Vector2d & operator+=(const Vector2d &other) {
			x += other.x;
			y += other.y;
			return *this;
		}

		Vector2d & operator-=(const Vector2d &other) {
			x -= other.x;
			y -= other.y;
			return *this;
		}

		Vector2d operator+(const Vector2d &other) const {
			return {x + other.x, y + other.y};
		}

		Vector2d operator-(const Vector2d &other) const {
			return {x - other.x, y - other.y};
		}

		template <Numeric N>
		Vector2d operator/(N divisor) const {
			return {x / divisor, y / divisor};
		}

		template <Numeric N>
		Vector2d operator*(N divisor) const {
			return {x * divisor, y * divisor};
		}

		/** Returns an angle in radians. */
		double atan2() const {
			return std::atan2(y, x);
		}
	};

	Buffer & operator+=(Buffer &, const Vector2d &);
	Buffer & operator<<(Buffer &, const Vector2d &);
	Buffer & operator>>(Buffer &, Vector2d &);

	struct Vector2i {
		int x = 0;
		int y = 0;

		double magnitude() const {
			return std::sqrt(static_cast<double>(x * x + y * y));
		}
	};

	Buffer & operator+=(Buffer &, const Vector2i &);
	Buffer & operator<<(Buffer &, const Vector2i &);
	Buffer & operator>>(Buffer &, Vector2i &);
}

template <>
struct std::formatter<Game3::Vector3> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &vector, auto &ctx) const {
		return std::format_to(ctx.out(), "({}, {}, {})", vector.x, vector.y, vector.z);
	}
};

template <>
struct std::formatter<Game3::Vector2d> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &vector, auto &ctx) const {
		return std::format_to(ctx.out(), "({}, {})", vector.x, vector.y);
	}
};

template <>
struct std::formatter<Game3::Vector2i> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &vector, auto &ctx) const {
		return std::format_to(ctx.out(), "({}, {})", vector.x, vector.y);
	}
};
