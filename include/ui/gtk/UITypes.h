#pragma once

#include "Types.h"

#include <memory>

#include <gtkmm.h>

namespace Game3 {
	class ClientInventory;

	struct DragSource {
		Slot slot;
		std::shared_ptr<ClientInventory> inventory;
	};
}
