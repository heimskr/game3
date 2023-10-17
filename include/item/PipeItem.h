#pragma once

#include "types/Directions.h"
#include "Log.h"
#include "types/Position.h"
#include "types/Quadrant.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/Item.h"
#include "realm/Realm.h"
#include "tileentity/Pipe.h"

namespace Game3 {
	std::shared_ptr<Item> getPipeItem(const Game &, PipeType);

	template <PipeType P>
	class PipeItem: public Item {
		protected:
			PipeItem(Identifier identifier_, const char *display_name, MoneyCount base_price):
				Item(std::move(identifier_), display_name, base_price, 64) {}

			virtual std::optional<bool> customUse(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>) {
				return std::nullopt;
			}

		public:
			bool use(Slot slot, ItemStack &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets) override{
				Realm &realm = *place.realm;
				const InventoryPtr inventory = place.player->getInventory();

				TileEntityPtr tile_entity = realm.tileEntityAt(place.position);

				if (!tile_entity) {
					if (modifiers.onlyShift())
						return false;

					inventory->decrease(stack, slot);

					auto pipe = TileEntity::create<Pipe>(place.position);
					pipe->setPresent(P, true);
					place.realm->add(pipe);

					// Hold ctrl while placing a pipe to prevent autopiping.
					if (!modifiers.onlyCtrl())
						pipe->autopipe(P);

					return true;
				}

				auto pipe = std::dynamic_pointer_cast<Pipe>(tile_entity);
				if (!pipe)
					return false;

				if (std::optional<bool> return_value = customUse(slot, stack, place, modifiers, offsets))
					return *return_value;

				if (modifiers.onlyShift()) {
					Game &game = place.getGame();

					bool self_present = false;
					bool others_present = false;

					for (const PipeType pipe_type: PIPE_TYPES) {
						if (pipe->getPresent(pipe_type)) {
							if (pipe_type == P) {
								self_present = true;
								place.player->give(ItemStack(game, getPipeItem(game, pipe_type), 1));
								pipe->setPresent(P, false);
							} else
								others_present = true;
						}
					}

					if (!others_present) {
						realm.queueDestruction(pipe);
					} else if (self_present) {
						pipe->increaseUpdateCounter();
						pipe->queueBroadcast();
					}

					return true;
				}

				const auto [x, y] = offsets;
				const Direction direction = toDirection(getQuadrant(x, y));

				if (!pipe->getPresent(P)) {
					inventory->decrease(stack, slot);
					pipe->setPresent(P, true);
					if (!modifiers.onlyCtrl())
						pipe->autopipe(P);
				} else if (modifiers.onlyCtrl()) {
					// Hold ctrl to toggle extractors.
					if (pipe->getDirections()[P][direction])
						pipe->toggleExtractor(P, direction);
				} else {
					pipe->toggle(P, direction);
				}

				pipe->increaseUpdateCounter();
				pipe->queueBroadcast();
				return true;
			}
	};

	class ItemPipeItem: public PipeItem<PipeType::Item> {
		public:
			static Identifier ID() { return {"base", "item/item_pipe"}; }
			ItemPipeItem(MoneyCount base_price):
				PipeItem(ID(), "Item Pipe", base_price) {}

		protected:
			std::optional<bool> customUse(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float> offsets) override;
	};

	class FluidPipeItem: public PipeItem<PipeType::Fluid> {
		public:
			static Identifier ID() { return {"base", "item/fluid_pipe"}; }
			FluidPipeItem(MoneyCount base_price):
				PipeItem(ID(), "Fluid Pipe", base_price) {}
	};

	class EnergyPipeItem: public PipeItem<PipeType::Energy> {
		public:
			static Identifier ID() { return {"base", "item/energy_pipe"}; }
			EnergyPipeItem(MoneyCount base_price):
				PipeItem(ID(), "Energy Pipe", base_price) {}
	};
}
