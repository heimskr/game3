#include "Tileset.h"
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
	Ore::Ore(Identifier identifier_, ItemStack stack_, Identifier tilename_, Identifier regen_tilename, float tooldown_multiplier, uint32_t max_uses, float cooldown_):
		NamedRegisterable(std::move(identifier_)),
		stack(std::move(stack_)),
		tilename(std::move(tilename_)),
		regenTilename(std::move(regen_tilename)),
		tooldownMultiplier(tooldown_multiplier),
		maxUses(max_uses),
		cooldown(cooldown_) {}

	Ore Ore::fromJSON(const Game &game, const nlohmann::json &json) {
		return {
			Identifier(), // Should be filled in later by the registry add methods
			ItemStack::fromJSON(game, json.at(0)),
			json.at(1),
			json.at(2),
			json.at(3),
			json.at(4),
			json.at(5)
		};
	}

	OreDeposit::OreDeposit(const Ore &ore, const Position &position_, float time_remaining, uint32_t uses_):
		TileEntity(ore.tilename, ID(), position_, true),
		oreType(ore.identifier),
		timeRemaining(time_remaining),
		uses(uses_) {}

	void OreDeposit::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["oreType"] = oreType;
		if (0.f < timeRemaining)
			json["timeRemaining"] = timeRemaining;
		if (uses != 0)
			json["uses"] = uses;
	}

	void OreDeposit::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		oreType = json.at("oreType");
		tileID = getOre(game).tilename;
		timeRemaining = json.contains("timeRemaining")? json.at("timeRemaining").get<float>() : 0.f;
		uses = json.contains("uses")? json.at("uses").get<uint32_t>() : 0;
	}

	void OreDeposit::tick(Game &, float delta) {
		timeRemaining = std::max(timeRemaining - delta, 0.f);
	}

	bool OreDeposit::onInteractNextTo(const PlayerPtr &player) {
		if (getSide() != Side::Server)
			return false;

		auto &inventory = *player->inventory;
		const Slot active_slot = inventory.activeSlot;
		if (auto *active_stack = inventory[active_slot]) {
			if (0.f < timeRemaining || 0.f < player->tooldown)
				return true;
			if (active_stack->hasAttribute("base:attribute/pickaxe"_id)) {
				const auto &tool = dynamic_cast<Tool &>(*active_stack->item);
				const Ore &ore = getOre(player->getGame());

				if (!inventory.add(ore.stack)) {
					player->tooldown = ore.tooldownMultiplier * tool.baseCooldown;

					if (ore.maxUses <= ++uses) {
						timeRemaining = ore.cooldown;
						uses = 0;
					}

					if (active_stack->reduceDurability())
						inventory.erase(active_slot);

					inventory.notifyOwner();
					increaseUpdateCounter();
					return true;
				}
			}
		}

		return false;
	}

	void OreDeposit::render(SpriteRenderer &sprite_renderer) {
		if (!isVisible())
			return;

		auto realm = getRealm();
		auto &tileset = realm->getTileset();

		if (tileID != tileset.getEmpty()) {
			const Ore &ore = getOre(realm->getGame());
			const auto tilesize = tileset.getTileSize();
			const TileID tile_id = tileset[0.f < timeRemaining? ore.regenTilename : tileID];
			const auto texture = tileset.getTexture(realm->getGame());
			const auto x = (tile_id % (*texture->width / tilesize)) * tilesize;
			const auto y = (tile_id / (*texture->width / tilesize)) * tilesize;
			sprite_renderer(*texture, {
				.x = static_cast<float>(position.column),
				.y = static_cast<float>(position.row),
				.x_offset = x / 2.f,
				.y_offset = y / 2.f,
				.size_x = static_cast<float>(tilesize),
				.size_y = static_cast<float>(tilesize),
			});
		}
	}

	const Ore & OreDeposit::getOre(const Game &game) const {
		return *game.registry<OreRegistry>().at(oreType);
	}

	void OreDeposit::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << oreType;
		buffer << timeRemaining;
		buffer << uses;
	}

	void OreDeposit::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> oreType;
		buffer >> timeRemaining;
		buffer >> uses;
	}
}
