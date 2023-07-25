#pragma once

#include "Directions.h"
#include "Log.h"
#include "Position.h"
#include "Quadrant.h"
#include "Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/Item.h"
#include "realm/Realm.h"
#include "tileentity/Pipe.h"

namespace Game3 {
	template <PipeType P>
	class PipeItem: public Item {
		protected:
			PipeItem(Identifier identifier_, const char *display_name, MoneyCount base_price):
				Item(std::move(identifier_), display_name, base_price, 64) {}

		public:

			bool use(Slot slot, ItemStack &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets) override{
				auto &realm = *place.realm;

				auto tile_entity = realm.tileEntityAt(place.position);
				if (!tile_entity) {
					if (modifiers.onlyShift())
						return false;

					if (--stack.count == 0)
						place.player->inventory->erase(slot);
					else
						place.player->inventory->notifyOwner();

					auto pipe = TileEntity::create<Pipe>(realm.getGame(), place.position);
					if (place.realm->add(pipe)) {
						pipe->setPresent(P, true);
						pipe->increaseUpdateCounter();
						pipe->broadcast();
					}

					return true;
				}

				auto pipe = std::dynamic_pointer_cast<Pipe>(tile_entity);
				if (!pipe)
					return false;

				if (modifiers.onlyShift()) {
					place.player->give(ItemStack(place.getGame(), shared_from_this(), 1));
					realm.queueDestruction(pipe);
					return true;
				}

				const auto [x, y] = offsets;
				const Direction direction = toDirection(getQuadrant(x, y));

				pipe->setPresent(P, true);

				// Hold ctrl to toggle extractors.
				if (modifiers.onlyCtrl()) {
					if (pipe->getDirections()[P][direction])
						pipe->toggleExtractor(P, direction);
				} else {
					pipe->toggle(P, direction);
				}

				pipe->increaseUpdateCounter();
				pipe->broadcast();
				return true;
			}
	};

	class ItemPipeItem: public PipeItem<PipeType::Item>   {
		public:
			ItemPipeItem(MoneyCount base_price):
				PipeItem("base:item/item_pipe"_id, "Item Pipe", base_price) {}
	};

	class FluidPipeItem: public PipeItem<PipeType::Fluid>  {
		public:
			FluidPipeItem(MoneyCount base_price):
				PipeItem("base:item/fluid_pipe"_id, "Fluid Pipe", base_price) {}
	};

	class EnergyPipeItem: public PipeItem<PipeType::Energy> {
		public:
			EnergyPipeItem(MoneyCount base_price):
				PipeItem("base:item/energy_pipe"_id, "Energy Pipe", base_price) {}
	};
}
