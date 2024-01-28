#include "game/ClientInventory.h"
#include "ui/gtk/DragSource.h"

namespace Game3 {
	ItemStack * DragSource::getStack() const {
		if (!inventory)
			return nullptr;

		return (*inventory)[slot];
	}
}
