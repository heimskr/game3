#include "game/ChunkPosition.h"

namespace Game3 {
	std::default_random_engine ChunkPosition::getRNG() const {
		return std::default_random_engine(static_cast<uint_fast32_t>(std::hash<ChunkPosition>()(*this)));
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

	void from_json(const nlohmann::json &json, ChunkRange &range) {
		range.topLeft = json.at(0);
		range.bottomRight = json.at(1);
	}

	void to_json(nlohmann::json &json, const ChunkRange &range) {
		json.push_back(range.topLeft);
		json.push_back(range.bottomRight);
	}
}
