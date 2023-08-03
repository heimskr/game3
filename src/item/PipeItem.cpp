#include "game/Game.h"
#include "item/PipeItem.h"

namespace Game3 {
	std::shared_ptr<Item> getPipeItem(const Game &game, PipeType pipe_type) {
		const auto &registry = game.registry<ItemRegistry>();
		switch (pipe_type) {
			case PipeType::Item:   return registry[ItemPipeItem::ID()];
			case PipeType::Fluid:  return registry[FluidPipeItem::ID()];
			case PipeType::Energy: return registry[EnergyPipeItem::ID()];
			default:
				throw std::invalid_argument("Unknown PipeType: " + std::to_string(int(pipe_type)));
		}
	}
}
