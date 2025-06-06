#include "item/Plantable.h"

namespace Game3 {
	bool Plantable::drag(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets, DragAction action) {
		if (action == DragAction::Start || action == DragAction::Update) {
			return use(slot, stack, place, modifiers, offsets);
		}

		return false;
	}
}
