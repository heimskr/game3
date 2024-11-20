#include "graphics/OpenGL.h"
#include "graphics/PathmapTextureCache.h"
#include "graphics/Texture.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"

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
			assert(realm != nullptr);
			ChunkRange(chunk_position).iterate([this](ChunkPosition neighbor) {
				if (auto iter = dataMap.find(neighbor); iter != dataMap.end()) {
					if (realm->getPathmapUpdateCounter(neighbor) > iter->second.pathUpdateCounter) {
						addChunk(neighbor, true);
					}
				} else {
					addChunk(neighbor, false);
				}
			});
			return;
		}

		lastChunk = chunk_position;
		std::erase_if(dataMap, [x = chunk_position.x, y = chunk_position.y](const std::pair<const ChunkPosition, ChunkData> &pair) {
			const auto [this_x, this_y] = pair.first;
			return std::abs(x - this_x) > 2 || std::abs(y - this_y) > 2;
		});

		ChunkRange(chunk_position).iterate([this](ChunkPosition neighbor) {
			addChunk(neighbor, false);
		});
	}

	void PathmapTextureCache::addChunk(ChunkPosition chunk_position, bool force) {
		assert(realm != nullptr);
		auto iter = dataMap.find(chunk_position);
		if (iter == dataMap.end()) {
			TexturePtr texture = generateTexture(chunk_position);
			if (texture != nullptr) {
				dataMap.try_emplace(chunk_position, std::move(texture), realm->getPathmapUpdateCounter(chunk_position));
			}
		} else if (force) {
			TexturePtr texture = generateTexture(chunk_position);
			if (texture != nullptr) {
				iter->second = {std::move(texture), realm->getPathmapUpdateCounter(chunk_position)};
			}
		}
	}

	TexturePtr PathmapTextureCache::getTexture(ChunkPosition chunk_position) const {
		if (auto iter = dataMap.find(chunk_position); iter != dataMap.end()) {
			return iter->second.texture;
		}

		return nullptr;
	}

	TexturePtr PathmapTextureCache::generateTexture(ChunkPosition chunk_position) {
		assert(realm != nullptr);

		Chunk<uint8_t> *chunk = realm->tileProvider.tryPathChunk(chunk_position);
		if (chunk == nullptr) {
			return nullptr;
		}

		TexturePtr texture = std::make_shared<Texture>(Identifier{}, false, -1);
		texture->format = GL_RED;

		auto lock = chunk->sharedLock();
		texture->init(*chunk, CHUNK_SIZE, CHUNK_SIZE);
		return texture;
	}
}
