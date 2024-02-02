#include <iostream>

#include "graphics/Texture.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/ServerInventory.h"
#include "graphics/SpriteRenderer.h"
#include "realm/Realm.h"
#include "tileentity/Crate.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	namespace {
		constexpr Identifier ITEM_NAME{"base", "te/crate"};
	}

	Crate::Crate(Identifier tile_id, const Position &position_, std::string name_):
		TileEntity(std::move(tile_id), ID(), position_, true), name(std::move(name_)) {}

	Crate::Crate(const Position &position_):
		Crate("base:tile/crate"_id, position_, "Crate") {}

	std::string Crate::getName() const {
		return name;
	}

	void Crate::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["name"] = name;
		auto lock = storedStack.sharedLock();
		if (storedStack)
			json["storedStack"] = *storedStack;
	}

	bool Crate::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, ItemStack *, Hand) {
		assert(getSide() == Side::Server);

		if (auto lock = storedStack.sharedLock(); storedStack) {
			INFO("Crate stack: {}", *storedStack);
		} else {
			INFO_("Crate has no stack.");
		}

		if (modifiers.onlyAlt()) {
			if (storedStack)
				storedStack->spawn(getRealm(), getPosition());
			queueDestruction();
			player->give(ItemStack(getGame(), ITEM_NAME));
		} else {
			// addObserver(player, false);
		}

		return true;
	}

	void Crate::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		assert(getSide() == Side::Server);
		name = json.at("name");
		if (auto iter = json.find("storedStack"); iter != json.end()) {
			storedStack = ItemStack::fromJSON(game, *iter);
			setInventoryStack();
		}
	}

	void Crate::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << name;
		buffer << storedStack;
	}

	void Crate::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);

		if (getSide() == Side::Server)
			setInventory(1);

		buffer >> name;
		buffer >> storedStack;

		if (getSide() == Side::Server)
			setInventoryStack();
	}

	bool Crate::mayInsertItem(const ItemStack &stack, Direction, Slot) {
		auto lock = storedStack.uniqueLock();
		if (!storedStack)
			return true;
		return storedStack->canMerge(stack);
	}

	bool Crate::mayExtractItem(Direction, Slot) {
		return !empty();
	}

	bool Crate::canInsertItem(const ItemStack &stack, Direction direction, Slot slot) {
		return mayInsertItem(stack, direction, slot);
	}

	bool Crate::canExtractItem(Direction, Slot) {
		return !empty();
	}

	std::optional<ItemStack> Crate::extractItem(Direction, bool remove, Slot) {
		auto stack_lock = storedStack.uniqueLock();

		if (remove) {
			{
				InventoryPtr inventory = getInventory(0);
				auto inventory_lock = inventory->uniqueLock();
				inventory->clear();
			}
			return std::move(storedStack.getBase());
		}

		return storedStack.getBase();
	}

	bool Crate::insertItem(const ItemStack &stack, Direction, std::optional<ItemStack> *leftover) {
		// Let's just hope we'll never reach the ItemCount cap.

		{
			auto lock = storedStack.uniqueLock();

			if (storedStack) {
				if (!storedStack->canMerge(stack))
					return false;
				storedStack->count += stack.count;
			} else {
				storedStack = stack;
			}

		}

		setInventoryStack();

		if (leftover)
			*leftover = std::nullopt;

		return true;
	}

	ItemCount Crate::itemsInsertable(const ItemStack &stack, Direction, Slot) {
		auto lock = storedStack.sharedLock();

		if (!storedStack)
			return stack.count;

		if (storedStack->canMerge(stack))
			return stack.count;

		return 0;
	}

	void Crate::iterateExtractableItems(Direction, const std::function<bool(const ItemStack &, Slot)> &function) {
		if (std::optional<ItemStack> stack = storedStack.copyBase())
			function(*stack, 0);
	}

	bool Crate::empty() const {
		auto lock = storedStack.sharedLock();
		return !storedStack || storedStack->count == 0;
	}

	void Crate::setInventoryStack() {
		InventoryPtr inventory = getInventory(0);
		if (!inventory)
			return;

		auto stack_lock = storedStack.sharedLock();
		auto inventory_lock = inventory->uniqueLock();
		if (storedStack)
			inventory->set(0, *storedStack);
		else
			inventory->clear();
	}

	void Crate::inventoryUpdated() {
		InventoriedTileEntity::inventoryUpdated();

		if (getSide() != Side::Server)
			return;

		absorbStackFromInventory();
	}

	void Crate::absorbStackFromInventory() {
		InventoryPtr inventory = getInventory(0);
		if (!inventory)
			return;

		auto stack_lock = storedStack.uniqueLock();
		auto inventory_lock = inventory->sharedLock();

		if (ItemStack *stack = (*inventory)[0])
			storedStack.getBase() = *stack;
		else
			storedStack.reset();
	}
}
