#pragma once

#include "game/Fluids.h"
#include "item/Item.h"

namespace Game3 {
	struct FluidRegistry;

	class FilledFlask: public Item {
		public:
			Identifier fluidName;

			FilledFlask(ItemID id_, std::string name_, MoneyCount base_price, Identifier fluid_name, ItemCount max_count = 64):
				Item(std::move(id_), std::move(name_), base_price, max_count), fluidName(std::move(fluid_name)) {}

			bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>, Hand) override;

			FluidStack getFluidStack(const Game &) const;
			FluidStack getFluidStack(const FluidRegistry &) const;
	};
}
