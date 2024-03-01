#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/CraftPacket.h"
#include "recipe/CraftingRecipe.h"

namespace Game3 {
	void CraftPacket::handle(const std::shared_ptr<ServerGame> &game, RemoteClient &client) {
		if (count == 0)
			return;

		auto player = client.getPlayer();
		if (!player)
			return;

		auto recipe = game->registry<CraftingRecipeRegistry>().maybe(recipeIndex);
		if (!recipe)
			return;

		const InventoryPtr inventory = player->getInventory(0);
		RealmPtr realm = player->getRealm();
		std::optional<std::vector<ItemStackPtr>> leftovers;

		for (size_t i = 0; i < count; ++i) {
			if (!recipe->craft(game, inventory, inventory, leftovers))
				break;

			if (leftovers) {
				for (const ItemStackPtr &leftover: *leftovers)
					leftover->spawn(player->getPlace());
				leftovers.reset();
			}
		}

		inventory->notifyOwner();
	}
}
