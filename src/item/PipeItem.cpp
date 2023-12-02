#include "game/Game.h"
#include "item/PipeItem.h"
#include "packet/OpenItemFiltersPacket.h"

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

	std::optional<bool> ItemPipeItem::customUse(Slot, ItemStack &, const Place &place, Modifiers modifiers, std::pair<float, float> offsets, Hand) {
		if (modifiers == Modifiers(true, true, false, false)) {
			PlayerPtr player = place.player;
			assert(player);

			const auto [x, y] = offsets;
			const Direction direction = toDirection(getQuadrant(x, y));

			player->send(OpenItemFiltersPacket(place.realm->id, place.position, direction));
			return true;
		}

		return std::nullopt;
	}
}
