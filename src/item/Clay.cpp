#include "item/Clay.h"
#include "item/Item.h"

namespace Game3 {
	std::optional<Landfill::Result> clayRequirement(const Place &place) {
		constexpr ItemCount COUNT = 10;
		switch (place.getLayer1()) {
			case Monomap::WATER:
				return Landfill::Result{ItemStack(Item::CLAY, COUNT), Monomap::SHALLOW_WATER};
			case Monomap::DEEP_WATER:
				return Landfill::Result{ItemStack(Item::CLAY, COUNT), Monomap::WATER};
			case Monomap::DEEPER_WATER:
				return Landfill::Result{ItemStack(Item::CLAY, COUNT), Monomap::DEEP_WATER};
			default:
				return std::nullopt;
		}
	}
}
