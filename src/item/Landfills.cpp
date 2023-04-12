#include "item/Landfills.h"
#include "item/Item.h"

namespace Game3 {
	std::optional<Landfill::Result> clayRequirement(const Place &place) {
		const Game &game = place.getGame();

		const auto &tilename = place.getLayer1Name();

		if (tilename == "base:tile/water"_id)
			return Landfill::Result{ItemStack(game, "base:item/clay"_id, Landfill::DEFAULT_COUNT), "base:tile/shallow_water"_id};

		if (tilename == "base:tile/deep_water"_id)
			return Landfill::Result{ItemStack(game, "base:item/clay"_id, Landfill::DEFAULT_COUNT), "base:tile/water"_id};

		if (tilename == "base:tile/deeper_water"_id)
			return Landfill::Result{ItemStack(game, "base:item/clay"_id, Landfill::DEFAULT_COUNT), "base:tile/deep_water"_id};

		return std::nullopt;
	}
}
