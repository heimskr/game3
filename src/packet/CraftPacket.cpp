#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/CraftPacket.h"
#include "recipe/CraftingRecipe.h"

namespace Game3 {
	void CraftPacket::handle(ServerGame &game, RemoteClient &client) {
		if (count == 0)
			return;

		auto player = client.getPlayer();
		if (!player)
			return;

		auto recipe = game.registry<CraftingRecipeRegistry>().maybe(recipeIndex);
		if (!recipe)
			return;

		auto inventory = player->inventory;
		auto realm = player->getRealm();
		std::vector<ItemStack> leftovers;

		for (size_t i = 0; i < count; ++i) {
			if (!recipe->craft(inventory, leftovers))
				break;

			for (const auto &leftover: leftovers)
				leftover.spawn(realm, player->position);

			leftovers.clear();
		}

		inventory->notifyOwner();
	}
}
