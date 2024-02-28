#include <iostream>

#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/RenderOptions.h"
#include "graphics/Tileset.h"
#include "item/Copier.h"
#include "realm/Realm.h"
#include "types/Position.h"

namespace Game3 {
	bool Copier::drag(Slot, ItemStack &stack, const Place &place, Modifiers) {
		auto lock = stack.data.uniqueLock();

		std::unordered_set<Position> positions;

		if (auto iter = stack.data.find("positions"); iter != stack.data.end())
			positions = *iter;

		if (auto iter = positions.find(place.position); iter != positions.end()) {
			positions.erase(iter);
		} else {
			positions.insert(place.position);
		}

		stack.data["positions"] = std::move(positions);
		place.player->getInventory(0)->notifyOwner();
		return true;
	}

	void Copier::renderEffects(const RendererContext &context, ItemStack &stack) const {
		RectangleRenderer &rectangle = context.rectangle;

		std::unordered_set<Position> positions;

		{
			auto lock = stack.data.sharedLock();
			if (auto iter = stack.data.find("positions"); iter != stack.data.end())
				positions = *iter;
		}

		for (const Position &position: positions) {
			rectangle.drawOnMap(RenderOptions{
				.x = double(position.column),
				.y = double(position.row),
				.sizeX = 1.,
				.sizeY = 1.,
				.color = {1.f, 1.f, 0.f, .5f},
			});
		}
	}
}
