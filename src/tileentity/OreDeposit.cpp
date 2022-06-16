#include "Tiles.h"
#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "realm/Realm.h"
#include "tileentity/OreDeposit.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/SpriteRenderer.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	void OreDeposit::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json[0] = type;
		json[1] = timeRemaining;
		json[2] = uses;
	}

	void OreDeposit::absorbJSON(const nlohmann::json &json) {
		TileEntity::absorbJSON(json);
		type = json.at(0);
		timeRemaining = json.at(1);
		uses = json.at(2);
	}

	void OreDeposit::tick(Game &, float delta) {
		timeRemaining = std::max(timeRemaining - delta, 0.f);
	}

	bool OreDeposit::onInteractNextTo(const std::shared_ptr<Player> &player) {
		if (0.f < timeRemaining)
			return false;

		auto &inventory = *player->inventory;
		const Slot active_slot = inventory.activeSlot;
		if (auto *active_stack = inventory[active_slot]) {
			if (active_stack->has(ItemAttribute::Pickaxe)) {
				auto &realm = *getRealm();
				if (!inventory.add({Item::WOOD, 1})) {
					realm.remove(shared_from_this());
					if (active_stack->reduceDurability())
						inventory.erase(active_slot);
					ItemCount saplings = 1;
					while (rand() % 4 == 1)
						++saplings;
					auto leftover = inventory.add({Item::SAPLING, saplings});
					if (leftover)
						realm.spawn<ItemEntity>(player->position, *leftover);
					inventory.notifyOwner();
					return true;
				}
			}
		}

		return false;
	}

	void OreDeposit::render(SpriteRenderer &sprite_renderer) {
		auto realm = getRealm();
		if (tileID != tileSets.at(realm->type)->getEmpty()) {
			auto &tilemap = *realm->tilemap2;
			const auto tilesize = tilemap.tileSize;
			const TileID tile_id = 0.f < timeRemaining? getRegenID(type) : tileID;
			const auto x = (tile_id % (tilemap.setWidth / tilesize)) * tilesize;
			const auto y = (tile_id / (tilemap.setWidth / tilesize)) * tilesize;
			sprite_renderer.drawOnMap(tilemap.texture, position.column, position.row, x / 2, y / 2, tilesize, tilesize);
		}
	}

	TileID OreDeposit::getID(Ore ore) {
		switch (ore) {
			case Ore::Coal:    return OverworldTiles::COAL_ORE;
			case Ore::Copper:  return OverworldTiles::COPPER_ORE;
			case Ore::Iron:    return OverworldTiles::IRON_ORE;
			case Ore::Gold:    return OverworldTiles::GOLD_ORE;
			case Ore::Diamond: return OverworldTiles::DIAMOND_ORE;
			default: return OverworldTiles::MISSING;
		}
	}

	TileID OreDeposit::getRegenID(Ore ore) {
		switch (ore) {
			case Ore::Coal:    return OverworldTiles::COAL_ORE_REGEN;
			case Ore::Copper:  return OverworldTiles::COPPER_ORE_REGEN;
			case Ore::Iron:    return OverworldTiles::IRON_ORE_REGEN;
			case Ore::Gold:    return OverworldTiles::GOLD_ORE_REGEN;
			case Ore::Diamond: return OverworldTiles::DIAMOND_ORE_REGEN;
			default: return OverworldTiles::MISSING;
		}
	}

	float OreDeposit::getTooldownMultiplier(Ore ore) {
		switch (ore) {
			case Ore::Coal:    return 1.f;
			case Ore::Copper:  return 1.f;
			case Ore::Iron:    return 1.25f;
			case Ore::Gold:    return 3.f;
			case Ore::Diamond: return 5.f;
			default: return 1.f;
		}
	}

	unsigned OreDeposit::getMaxUses(Ore ore) {
		switch (ore) {
			case Ore::Coal:    return 48;
			case Ore::Copper:  return 32;
			case Ore::Iron:    return 32;
			case Ore::Gold:    return 16;
			case Ore::Diamond: return 4;
			default: return 1;
		}
	}

	ItemStack OreDeposit::getOreStack(ItemCount count) {
		switch (type) {
			case Ore::Coal:    return ItemStack(Item::COAL,        count);
			case Ore::Copper:  return ItemStack(Item::COPPER_ORE,  count);
			case Ore::Iron:    return ItemStack(Item::IRON_ORE,    count);
			case Ore::Gold:    return ItemStack(Item::GOLD_ORE,    count);
			case Ore::Diamond: return ItemStack(Item::DIAMOND_ORE, count);
			default: throw std::invalid_argument("Invalid ore type: " + std::to_string(static_cast<int>(type)));
		}
	}
}
