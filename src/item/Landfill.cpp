#include "Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "item/Landfill.h"
#include "realm/Realm.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"

namespace Game3 {
	Landfill::Landfill(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count, RequirementFn requirement_):
		Item(std::move(id_), std::move(name_), base_price, max_count),
		requirement(std::move(requirement_)) {}

	Landfill::Landfill(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count, Identifier tileset_name, Identifier required_tile, ItemStack requirement, Identifier new_tile):
		Landfill(std::move(id_), std::move(name_), base_price, max_count, [=](const Place &place) -> std::optional<Result> {
			if (place.realm->tilemap1->tileset->identifier == tileset_name && place.getLayer1Name() == required_tile)
				return Result(requirement, new_tile);
			return std::nullopt;
		}) {}

	Landfill::Landfill(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count, Identifier tileset_name, Identifier required_tile, ItemCount required_count, Identifier new_tile):
		Item(std::move(id_), std::move(name_), base_price, max_count),
		tilesetName(std::move(tileset_name)),
		requiredTile(std::move(required_tile)),
		requiredCount(required_count),
		newTile(std::move(new_tile)) {}

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
		if (!requirement && requiredTile && requiredCount != static_cast<ItemCount>(-1) && newTile) {
			requirement = [this](const Place &place) -> std::optional<Result> {
				if (place.getLayer1Name() == requiredTile)
					return Result(ItemStack(shared_from_this(), requiredCount), newTile);
				return std::nullopt;
			};
		}

		return requirement.has_value();
	}
}
