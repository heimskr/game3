#include "Tiles.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "item/Landfill.h"
#include "realm/Realm.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"

namespace Game3 {
	Landfill::Landfill(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count, RequirementFn requirement_):
		Item(id_, std::move(name_), base_price, max_count),
		requirement(std::move(requirement_)) {}

	Landfill::Landfill(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count, TileID required_tile, ItemStack requirement, TileID new_tile):
		Landfill(id_, std::move(name_), base_price, max_count, [=](const Place &place) -> std::optional<Result> {
			if (place.getLayer1() == required_tile)
				return Result(requirement, new_tile);
			return std::nullopt;
		}) {}

	Landfill::Landfill(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count, TileID required_tile, ItemCount required_count, TileID new_tile):
		Item(id_, std::move(name_), base_price, max_count),
		requiredTile(required_tile),
		requiredCount(required_count),
		newTile(new_tile) {}

	bool Landfill::use(Slot slot, ItemStack &stack, const Place &place) {
		if (!fixRequirement())
			return false;

		auto &player = *place.player;
		auto &realm  = *place.realm;

		if (const auto result = callRequirement(place)) {
			if (result->required <= stack) {
				if ((stack.count -= result->required.count) == 0)
					player.inventory->erase(slot);
				realm.setLayer1(place.position, result->newTile);
				realm.getGame().canvas.window.activateContext();
				realm.reupload();
				player.inventory->notifyOwner();
				return true;
			}
		}

		return false;
	}

	std::optional<Landfill::Result> Landfill::callRequirement(const Place &place) {
		if (!fixRequirement())
			return std::nullopt;
		return (*requirement)(place);
	}

	bool Landfill::fixRequirement() {
		if (!requirement && requiredTile != static_cast<TileID>(-1) && requiredCount != static_cast<ItemCount>(-1) && newTile != static_cast<TileID>(-1)) {
			requirement = [this, stack = ItemStack(shared_from_this(), requiredCount)](const Place &place) -> std::optional<Result> {
				if (place.getLayer1() == requiredTile)
					return Result(stack, newTile);
				return std::nullopt;
			};
		}

		return requirement.has_value();
	}
}
