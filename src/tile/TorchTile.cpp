#include "Log.h"
#include "types/Position.h"
#include "graphics/CircleRenderer.h"
#include "graphics/RendererSet.h"
#include "graphics/RenderOptions.h"
#include "graphics/Tileset.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "tile/TorchTile.h"
#include "util/Util.h"

namespace Game3 {
	TorchTile::TorchTile():
		Tile(ID()) {}

	void TorchTile::renderStaticLighting(const Place &place, Layer, const RendererSet &renderers) {
		const auto [row, column] = place.position;
		constexpr static float radius = 16;

		renderers.circle.drawOnMap(RenderOptions{
			.x = column + .5f,
			.y = row + .5f,
			.sizeX = radius,
			.sizeY = radius,
			.color = {1, 1, 0.5, 1},
		}, 0.7);
	}
}
