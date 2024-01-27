#include "game/TileProvider.h"
#include "net/Buffer.h"
#include "realm/Realm.h"
#include "types/ChunkPosition.h"
#include "util/Util.h"

namespace Game3 {
	ChunkPosition::ChunkPosition(IntType x_, IntType y_):
		x(x_), y(y_) {}

	ChunkPosition::ChunkPosition(const Position &position):
		x(TileProvider::divide<IntType>(position.column)),
		y(TileProvider::divide<IntType>(position.row)) {}

	ChunkPosition::ChunkPosition(std::string_view string) {
		if (string.size() < 6 || string.front() != '(' || string.back() != ')')
			throw std::invalid_argument("Invalid ChunkPosition string");

		string.remove_prefix(1);
		string.remove_suffix(1);

		size_t comma_size = 2;
		size_t comma = string.find(", ");

		if (comma == std::string_view::npos) {
			comma = string.find(',');
			comma_size = 1;
			if (comma == std::string_view::npos)
				throw std::invalid_argument("Invalid ChunkPosition string");
		}

		x = parseNumber<IntType>(string.substr(0, comma));
		y = parseNumber<IntType>(string.substr(comma + comma_size));
	}

	std::default_random_engine ChunkPosition::getRNG() const {
		return std::default_random_engine(static_cast<uint_fast32_t>(std::hash<ChunkPosition>()(*this)));
	}

	Buffer & operator+=(Buffer &buffer, const ChunkPosition &chunk_position) {
		return (buffer += chunk_position.x) += chunk_position.y;
	}

	Buffer & operator<<(Buffer &buffer, const ChunkPosition &chunk_position) {
		return buffer << chunk_position.x << chunk_position.y;
	}

	Buffer & operator>>(Buffer &buffer, ChunkPosition &chunk_position) {
		return buffer >> chunk_position.x >> chunk_position.y;
	}

	ChunkPosition ChunkPosition::operator+(const ChunkPosition &other) const {
		return {x + other.x, y + other.y};
	}

	ChunkPosition ChunkPosition::operator-(const ChunkPosition &other) const {
		return {x - other.x, y - other.y};
	}

	ChunkPosition & ChunkPosition::operator+=(const ChunkPosition &other) {
		x += other.x;
		y += other.y;
		return *this;
	}

	ChunkPosition & ChunkPosition::operator-=(const ChunkPosition &other) {
		x -= other.x;
		y -= other.y;
		return *this;
	}

	ChunkPosition::operator std::string() const {
		return '(' + std::to_string(x) + ", " + std::to_string(y) + ')';
	}

	void from_json(const nlohmann::json &json, ChunkPosition &position) {
		position.x = json.at(0);
		position.y = json.at(1);
	}

	void to_json(nlohmann::json &json, const ChunkPosition &position) {
		json.push_back(position.x);
		json.push_back(position.y);
	}

	ChunkRange::ChunkRange(ChunkPosition top_left, ChunkPosition bottom_right):
		topLeft(top_left), bottomRight(bottom_right) {}

	ChunkRange::ChunkRange(ChunkPosition chunk_position):
		topLeft    {chunk_position.x - ChunkPosition::IntType(REALM_DIAMETER / 2), chunk_position.y - ChunkPosition::IntType(REALM_DIAMETER / 2)},
		bottomRight{chunk_position.x + ChunkPosition::IntType(REALM_DIAMETER / 2), chunk_position.y + ChunkPosition::IntType(REALM_DIAMETER / 2)} {}

	ChunkRange::operator std::string() const {
		return '[' + std::string(topLeft) + ", " + std::string(bottomRight) + ']';
	}

	void from_json(const nlohmann::json &json, ChunkRange &range) {
		range.topLeft = json.at(0);
		range.bottomRight = json.at(1);
	}

	void to_json(nlohmann::json &json, const ChunkRange &range) {
		json.push_back(range.topLeft);
		json.push_back(range.bottomRight);
	}
}
