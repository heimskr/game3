#include "math/Vector.h"
#include "net/Buffer.h"
#include "types/Position.h"

namespace Game3 {
	template <>
	std::string BasicBuffer::getType(const Vector3 &, bool) {
		return std::string{'\x32'} + getType(float{}, false);
	}

	Buffer & operator+=(Buffer &buffer, const Vector3 &vector) {
		return ((buffer.appendType(vector, false) += vector.x) += vector.y) += vector.z;
	}

	Buffer & operator<<(Buffer &buffer, const Vector3 &vector) {
		return buffer += vector;
	}

	BasicBuffer & operator>>(BasicBuffer &buffer, Vector3 &vector) {
		const auto type = buffer.popType();
		if (!Buffer::typesMatch(type, buffer.getType(vector, false))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected shortlist<f32, 3> for Vector3)");
		}
		popBuffer(buffer, vector.x);
		popBuffer(buffer, vector.y);
		popBuffer(buffer, vector.z);
		return buffer;
	}

	Vector2d::Vector2d(Position position):
		x(position.column), y(position.row) {}

	template <>
	std::string BasicBuffer::getType(const Vector2d &, bool) {
		return std::string{'\x31'} + getType(double{}, false);
	}

	Buffer & operator+=(Buffer &buffer, const Vector2d &vector) {
		return (buffer.appendType(vector, false) += vector.x) += vector.y;
	}

	Buffer & operator<<(Buffer &buffer, const Vector2d &vector) {
		return buffer += vector;
	}

	BasicBuffer & operator>>(BasicBuffer &buffer, Vector2d &vector) {
		const auto type = buffer.popType();
		if (!Buffer::typesMatch(type, buffer.getType(vector, false))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected shortlist<f64, 2> for Vector2d)");
		}
		popBuffer(buffer, vector.x);
		popBuffer(buffer, vector.y);
		return buffer;
	}

	template <>
	std::string BasicBuffer::getType(const Vector2i &, bool) {
		return std::string{'\x31'} + getType(decltype(Vector2i::x){}, false);
	}

	Buffer & operator+=(Buffer &buffer, const Vector2i &vector) {
		return (buffer.appendType(vector, false) += vector.x) += vector.y;
	}

	Buffer & operator<<(Buffer &buffer, const Vector2i &vector) {
		return buffer += vector;
	}

	BasicBuffer & operator>>(BasicBuffer &buffer, Vector2i &vector) {
		const auto type = buffer.popType();
		if (!Buffer::typesMatch(type, buffer.getType(vector, false))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected shortlist<i32, 2> for Vector2i)");
		}
		popBuffer(buffer, vector.x);
		popBuffer(buffer, vector.y);
		return buffer;
	}
}
