#pragma once

#include "types/Types.h"

#include <memory>

#include <gtkmm.h>

namespace Game3 {
	class ClientInventory;
	class ItemStack;

	struct DragSource {
		Slot slot;
		std::shared_ptr<ClientInventory> inventory;
		InventoryID index;
		ItemStackPtr getStack() const;
	};
}
