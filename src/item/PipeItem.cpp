#include "game/Game.h"
#include "item/PipeItem.h"
#include "packet/OpenItemFiltersPacket.h"

namespace Game3 {
	std::shared_ptr<Item> getPipeItem(const Game &game, Substance pipe_type) {
		const auto &registry = game.registry<ItemRegistry>();
		switch (pipe_type) {
			case Substance::Item:   return registry[ItemPipeItem::ID()];
			case Substance::Fluid:  return registry[FluidPipeItem::ID()];
			case Substance::Energy: return registry[EnergyPipeItem::ID()];
			case Substance::Data:   return registry[DataPipeItem::ID()];
			default:
				throw std::invalid_argument("Unknown Substance: " + std::to_string(int(pipe_type)));
		}
	}

	std::optional<bool> ItemPipeItem::customUse(Slot, const ItemStackPtr &, const Place &place, Modifiers modifiers, std::pair<float, float> offsets) {
		if (modifiers == Modifiers(true, true, false, false)) {
			PlayerPtr player = place.player;
			assert(player);

			const auto [x, y] = offsets;
			const Direction direction = toDirection(getQuadrant(x, y));

			player->send(make<OpenItemFiltersPacket>(place.realm->id, place.position, direction));
			return true;
		}

		return std::nullopt;
	}
}
