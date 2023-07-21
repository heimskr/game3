#include <cassert>

#include "Log.h"
#include "entity/Entity.h"
#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "entity/ServerPlayer.h"
#include "game/Agent.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "net/Buffer.h"
#include "packet/InventoryPacket.h"
#include "packet/SetActiveSlotPacket.h"
#include "packet/ActiveSlotSetPacket.h"
#include "packet/DropItemPacket.h"
#include "realm/Realm.h"
#include "recipe/CraftingRecipe.h"
#include "util/Util.h"

namespace Game3 {
	Inventory::Inventory(std::shared_ptr<Agent> owner, Slot slot_count, Slot active_slot):
		weakOwner(owner), slotCount(slot_count), activeSlot(active_slot) {}

	Inventory::Inventory(const Inventory &other):
		weakOwner(other.weakOwner),
		slotCount(other.slotCount.load()),
		activeSlot(other.activeSlot.load()) {}

	Inventory::Inventory(Inventory &&other):
	weakOwner(other.weakOwner),
	slotCount(other.slotCount.load()),
	activeSlot(other.activeSlot.load()) {
		other.weakOwner = {};
		other.slotCount = 0;
		other.activeSlot = 0;
	}

	Inventory & Inventory::operator=(const Inventory &other) {
		weakOwner = other.weakOwner;
		slotCount = other.slotCount.load();
		activeSlot = other.activeSlot.load();
		return *this;
	}

	Inventory & Inventory::operator=(Inventory &&other) {
		weakOwner = std::move(other.weakOwner);
		slotCount = other.slotCount.exchange(0);
		activeSlot = other.activeSlot.exchange(0);
		return *this;
	}

	void Inventory::erase(bool suppress_notification) {
		erase(activeSlot, suppress_notification);
	}

	std::shared_ptr<Agent> Inventory::getOwner() const {
		if (auto owner = weakOwner.lock())
			return owner;
		throw std::runtime_error("Couldn't lock inventory owner");
	}

	void Inventory::prevSlot() {
		if (0 < activeSlot)
			setActive(activeSlot - 1, false);
	}

	void Inventory::nextSlot() {
		if (activeSlot < slotCount - 1)
			setActive(activeSlot + 1, false);
	}

	ItemCount Inventory::craftable(const CraftingRecipe &recipe) const {
		ItemCount out = UINT64_MAX;

		for (const auto &input: recipe.input) {
			if (input.is<ItemStack>()) {
				const ItemStack &stack = input.get<ItemStack>();
				out = std::min(out, count(stack) / stack.count);
			} else {
				const auto &[attribute, attribute_count] = input.get<AttributeRequirement>();
				out = std::min(out, countAttribute(attribute) / attribute_count);
			}
		}

		return out;
	}
}
