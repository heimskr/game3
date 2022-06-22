#include "Tiles.h"
#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "item/Tool.h"
#include "realm/Realm.h"
#include "tileentity/OreDeposit.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/SpriteRenderer.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	void OreDeposit::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["type"] = type;
		if (0.f < timeRemaining)
			json["timeRemaining"] = timeRemaining;
		if (uses != 0)
			json["uses"] = uses;
	}

	void OreDeposit::absorbJSON(const nlohmann::json &json) {
		TileEntity::absorbJSON(json);
		type = json.at("type");
		timeRemaining = json.contains("timeRemaining")? json.at("timeRemaining").get<float>() : 0.f;
		uses = json.contains("uses")? json.at("uses").get<unsigned>() : 0;
	}

	void OreDeposit::tick(Game &, float delta) {
		timeRemaining = std::max(timeRemaining - delta, 0.f);
	}

	bool OreDeposit::onInteractNextTo(const std::shared_ptr<Player> &player) {
		auto &inventory = *player->inventory;
		const Slot active_slot = inventory.activeSlot;
		if (auto *active_stack = inventory[active_slot]) {
			if (0.f < timeRemaining || 0.f < player->tooldown)
				return true;
			if (active_stack->has(ItemAttribute::Pickaxe)) {
				const auto &tool = dynamic_cast<Tool &>(*active_stack->item);
				if (!inventory.add(getOreStack())) {
					player->tooldown = tooldownMultiplier * tool.baseCooldown;

					if (maxUses <= ++uses) {
						timeRemaining = cooldown;
						uses = 0;
					}

					if (active_stack->reduceDurability())
						inventory.erase(active_slot);

					inventory.notifyOwner();
					return true;
				}
			}
		}

		return false;
	}

	void OreDeposit::render(SpriteRenderer &sprite_renderer) {
		auto &realm = *getRealm();
		if (tileID != tileSets.at(realm.type)->getEmpty()) {
			auto &tilemap = *realm.tilemap2;
			const auto tilesize = tilemap.tileSize;
			const TileID tile_id = 0.f < timeRemaining? getRegenID(type) : tileID;
			const auto x = (tile_id % (tilemap.setWidth / tilesize)) * tilesize;
			const auto y = (tile_id / (tilemap.setWidth / tilesize)) * tilesize;
			sprite_renderer.drawOnMap(tilemap.texture, position.column, position.row, x / 2, y / 2, tilesize, tilesize);
		}
	}

	TileID OreDeposit::getID(Ore ore) {
		switch (ore) {
			case Ore::Coal:    return Monomap::COAL_ORE;
			case Ore::Copper:  return Monomap::COPPER_ORE;
			case Ore::Iron:    return Monomap::IRON_ORE;
			case Ore::Gold:    return Monomap::GOLD_ORE;
			case Ore::Diamond: return Monomap::DIAMOND_ORE;
			default: return Monomap::MISSING;
		}
	}

	TileID OreDeposit::getRegenID(Ore ore) {
		switch (ore) {
			case Ore::Coal:    return Monomap::COAL_ORE_REGEN;
			case Ore::Copper:  return Monomap::COPPER_ORE_REGEN;
			case Ore::Iron:    return Monomap::IRON_ORE_REGEN;
			case Ore::Gold:    return Monomap::GOLD_ORE_REGEN;
			case Ore::Diamond: return Monomap::DIAMOND_ORE_REGEN;
			default: return Monomap::MISSING;
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
			case Ore::Coal:    return 16;
			case Ore::Copper:  return 16;
			case Ore::Iron:    return 16;
			case Ore::Gold:    return 8;
			case Ore::Diamond: return 2;
			default: return 1;
		}
	}

	float OreDeposit::getCooldown(Ore ore) {
		switch (ore) {
			case Ore::Coal:    return 5.f;
			case Ore::Copper:  return 5.f;
			case Ore::Iron:    return 10.f;
			case Ore::Gold:    return 30.f;
			case Ore::Diamond: return 60.f;
			default: return 0.f;
		}
	}

	ItemStack OreDeposit::getOreStack(ItemCount count) {
		return getOreStack(type, count);
	}

	ItemStack OreDeposit::getOreStack(Ore ore, ItemCount count) {
		switch (ore) {
			case Ore::Coal:    return ItemStack(Item::COAL,        count);
			case Ore::Copper:  return ItemStack(Item::COPPER_ORE,  count);
			case Ore::Iron:    return ItemStack(Item::IRON_ORE,    count);
			case Ore::Gold:    return ItemStack(Item::GOLD_ORE,    count);
			case Ore::Diamond: return ItemStack(Item::DIAMOND_ORE, count);
			default: throw std::invalid_argument("Invalid ore type: " + std::to_string(static_cast<int>(ore)));
		}
	}
}
