#include "game/Game.h"
#include "item/Food.h"
#include "tile/FoodTile.h"
#include "types/Position.h"

namespace Game3 {
	FoodTile::FoodTile(Identifier tilename, Identifier food_item_name):
		Tile(std::move(tilename)), foodItemName(std::move(food_item_name)) {}

	bool FoodTile::interact(const Place &place, Layer layer, const ItemStackPtr &, Hand) {
		GamePtr game = place.getGame();
		PlayerPtr player = place.player;

		auto food = std::dynamic_pointer_cast<Food>(game->registry<ItemRegistry>()[foodItemName]);
		if (!food) {
			throw std::logic_error(std::format("Can't find food item \"{}\"", foodItemName));
		}

		player->heal(food->getHealedPoints(player));
		place.set(layer, 0);
		return true;
	}
}
