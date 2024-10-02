#include "Log.h"
#include "entity/Player.h"
#include "graphics/CircleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/RenderOptions.h"
#include "graphics/Tileset.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "tile/TorchTile.h"
#include "types/Position.h"
#include "util/Util.h"

namespace Game3 {
	TorchTile::TorchTile():
		Tile(ID()) {}

	void TorchTile::renderStaticLighting(const Place &place, Layer, const RendererContext &context) {
		assert(place.realm->isClient());
		const size_t tile_size = place.realm->getTileset().getTileSize();
		const float radius = 64 * tile_size;
		const float x_factor = 1;
		const float y_factor = 1;

		const ChunkPosition chunk = place.player->getChunk() - ChunkPosition(1, 1);
		auto [top, left] = place.position - chunk.topLeft();

		left *= tile_size * x_factor;
		left += 8 * x_factor;
		top  *= tile_size * y_factor;
		top  += 8 * y_factor;

		context.circle.drawOnScreen(Color{1, 1, 0.5, 1}, left, top, radius, radius, 0.2);
	}
}
