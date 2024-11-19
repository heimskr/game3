#include "graphics/OpenGL.h"
#include "graphics/PathmapTextureCache.h"
#include "graphics/Texture.h"
#include "realm/Realm.h"

#include <cassert>

namespace Game3 {
	PathmapTextureCache::PathmapTextureCache() = default;

	void PathmapTextureCache::updateRealm(const std::shared_ptr<Realm> &new_realm) {
		if (realm != new_realm) {
			realm = new_realm;
			reset();
		}
	}

	void PathmapTextureCache::reset() {
		dataMap.clear();
		lastChunk.reset();
	}

	void PathmapTextureCache::visitChunk(ChunkPosition chunk_position) {
		if (lastChunk == chunk_position) {
			return;
		}

		lastChunk = chunk_position;
		std::erase_if(dataMap, [x = chunk_position.x, y = chunk_position.y](const std::pair<const ChunkPosition, ChunkData> &pair) {
			const auto [this_x, this_y] = pair.first;
			return std::abs(x - this_x) > 2 || std::abs(y - this_y) > 2;
		});

		ChunkRange(chunk_position).iterate([this](ChunkPosition neighbor) {
			addChunk(neighbor);
		});
	}

	void PathmapTextureCache::addChunk(ChunkPosition chunk_position) {
		assert(realm != nullptr);
		dataMap[chunk_position] = {generateTexture(chunk_position), realm->getPathmapUpdateCounter(chunk_position)};
	}

	TexturePtr PathmapTextureCache::generateTexture(ChunkPosition chunk_position) {
		assert(realm != nullptr);
		TexturePtr texture = std::make_shared<Texture>(Identifier{}, false, -1);
		texture->format = GL_RED;

		const Chunk<uint8_t> &chunk = realm->tileProvider.getPathChunk(chunk_position);
		auto lock = chunk.sharedLock();
		texture->init(chunk, CHUNK_SIZE, CHUNK_SIZE);
		return texture;
	}
}
