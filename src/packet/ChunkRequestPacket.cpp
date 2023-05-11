#include "packet/ChunkRequestPacket.h"
#include "packet/PacketError.h"

namespace Game3 {
	void ChunkRequestPacket::encode(Game &, Buffer &buffer) const {
		std::vector<decltype(ChunkPosition::x)> coordinates;
		coordinates.reserve(2 * chunkPositions.size());

		for (const auto [x, y]: chunkPositions) {
			coordinates.push_back(x);
			coordinates.push_back(y);
		}

		buffer << realmID << coordinates;
	}

	void ChunkRequestPacket::decode(Game &, Buffer &buffer) {
		std::vector<decltype(ChunkPosition::x)> coordinates;

		buffer >> realmID >> coordinates;

		if (coordinates.empty())
			throw PacketError("Empty ChunkRequestPacket");

		if (coordinates.size() % 2 != 0)
			throw PacketError("Misshapen ChunkRequestPacket");

		if (coordinates.size() > 128)
			throw PacketError("Excessively greedy ChunkRequestPacket");

		chunkPositions.clear();
		for (size_t i = 0; i < coordinates.size(); i += 2)
			chunkPositions.insert(ChunkPosition{coordinates[i], coordinates[i + 1]});
	}
}
