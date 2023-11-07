#include "game/Inventory.h"
#include "graphics/Tileset.h"
#include "item/Wrench.h"
#include "realm/Realm.h"
#include "tileentity/Pipe.h"
#include "types/Position.h"

namespace Game3 {
	Wrench::Wrench(ItemID id_, std::string name_, MoneyCount base_price):
	Item(id_, std::move(name_), base_price, 1) {
		attributes.insert("base:attribute/wrench");
	}

	bool Wrench::use(Slot, ItemStack &, const Place &place, Modifiers modifiers, std::pair<float, float>) {
		Realm &realm = *place.realm;

		if (auto pipe = std::dynamic_pointer_cast<Pipe>(realm.tileEntityAt(place.position))) {
			PipeType pipe_type{};

			if (modifiers.empty())
				pipe_type = PipeType::Item;
			else if (modifiers.onlyShift())
				pipe_type = PipeType::Fluid;
			else
				return false;

			pipe->toggleMode(pipe_type);
			return true;
		}

		return false;
	}
}
