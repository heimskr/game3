#pragma once

#include "types/ChunkPosition.h"

#include <memory>
#include <optional>
#include <unordered_map>

namespace Game3 {
	class Realm;
	class Texture;

	/** To render water caustic patterns in the undersea realm, we use a texture derived from the realm's pathmap.
	 *  We want to produce these textures on demand and cache them to some extent. */
	class PathmapTextureCache {
		public:
			PathmapTextureCache();

			/** Resets the data if the realm provided is different from the last used realm. */
			void updateRealm(const std::shared_ptr<Realm> &);

			void reset();

			void visitChunk(ChunkPosition);

			void addChunk(ChunkPosition, bool force);

			/** Will return nullptr if the chunk position isn't in the cache. */
			std::shared_ptr<Texture> getTexture(ChunkPosition) const;

			std::shared_ptr<Texture> generateTexture(ChunkPosition);

		private:
			struct ChunkData {
				std::shared_ptr<Texture> texture{};
				uint64_t pathUpdateCounter{};
			};

			std::shared_ptr<Realm> realm;
			std::optional<ChunkPosition> lastChunk;
			std::unordered_map<ChunkPosition, ChunkData> dataMap;
	};
}
