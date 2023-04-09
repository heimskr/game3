#include "item/Landfills.h"
#include "item/Item.h"

namespace Game3 {
	std::optional<Landfill::Result> clayRequirement(const Place &place) {
		const Game &game = place.getGame();

		const auto &tilename = place.getLayer1Name();

		if (tilename == "base:tile/water")
			return Landfill::Result{ItemStack(game, "base:clay", Landfill::DEFAULT_COUNT), "base:tile/shallow_water"};

		if (tilename == "base:tile/deep_water")
			return Landfill::Result{ItemStack(game, "base:clay", Landfill::DEFAULT_COUNT), "base:tile/water"};

		if (tilename == "base:tile/deeper_water")
			return Landfill::Result{ItemStack(game, "base:clay", Landfill::DEFAULT_COUNT), "base:tile/deep_water"};

		return std::nullopt;
	}
}
