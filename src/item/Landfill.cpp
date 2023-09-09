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

	Landfill::Landfill(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count, const Identifier &tileset_name, const Identifier &required_tile, const ItemStack &requirement, const Identifier &new_tile):
		Landfill(std::move(id_), std::move(name_), base_price, max_count, [=](const Place &place) -> std::optional<Result> {
			if (place.realm->getTileset().identifier == tileset_name && place.getName(Layer::Terrain) == required_tile)
				return Result{requirement, new_tile};
			return std::nullopt;
		}) {}

	Landfill::Landfill(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count, Identifier tileset_name, Identifier required_tile, ItemCount required_count, Identifier new_tile):
		Item(std::move(id_), std::move(name_), base_price, max_count),
		tilesetName(std::move(tileset_name)),
		requiredTile(std::move(required_tile)),
		requiredCount(required_count),
		newTile(std::move(new_tile)) {}

	bool Landfill::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>) {
		if (!fixRequirement())
			return false;

		auto &player = *place.player;
		auto &realm  = *place.realm;

		if (const auto result = callRequirement(place)) {
			if (result->required <= stack) {
				stack.count -= result->required.count;
				if (stack.count == 0)
					player.getInventory()->erase(slot);
				realm.setTile(Layer::Terrain, place.position, result->newTile);
				realm.reupload();
				player.getInventory()->notifyOwner();
				return true;
			}
		}

		return false;
	}

	std::optional<Landfill::Result> Landfill::callRequirement(const Place &place) {
		if (!fixRequirement())
			return std::nullopt;
		assert(requirement);
		return (*requirement)(place);
	}

	bool Landfill::fixRequirement() {
		if (!requirement && requiredTile && requiredCount != static_cast<ItemCount>(-1) && newTile) {
			requirement = [this](const Place &place) -> std::optional<Result> {
				if (place.getName(Layer::Terrain) == requiredTile)
					return Result{ItemStack(place.getGame(), shared_from_this(), requiredCount), newTile};
				return std::nullopt;
			};
		}

		return requirement.has_value();
	}
}
