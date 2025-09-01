#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "item/Tool.h"
#include "lib/JSON.h"
#include "realm/Realm.h"
#include "tileentity/OreDeposit.h"
#include "ui/Window.h"

namespace Game3 {
	Ore::Ore(Identifier identifier_, ItemStackPtr stack_, Identifier tilename_, Identifier regen_tilename, float tooldown_multiplier, uint32_t max_uses, float cooldown_):
		NamedRegisterable(std::move(identifier_)),
		stack(std::move(stack_)),
		tilename(std::move(tilename_)),
		regenTilename(std::move(regen_tilename)),
		tooldownMultiplier(tooldown_multiplier),
		maxUses(max_uses),
		cooldown(cooldown_) {}

	Ore Ore::fromJSON(const GamePtr &game, const boost::json::value &json) {
		return {
			Identifier(), // Should be filled in later by the registry add methods
			boost::json::value_to<ItemStackPtr>(json.at(0), game),
			boost::json::value_to<Identifier>(json.at(1)),
			boost::json::value_to<Identifier>(json.at(2)),
			getNumber<float>(json.at(3)),
			getNumber<uint32_t>(json.at(4)),
			getNumber<float>(json.at(5)),
		};
	}

	OreDeposit::OreDeposit(const Ore &ore, const Position &position_, uint32_t uses_):
		TileEntity(ore.tilename, ID(), position_, true),
		oreType(ore.identifier),
		uses(uses_) {}

	void OreDeposit::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		auto &object = json.as_object();
		object["oreType"] = boost::json::value_from(oreType);
		object["ready"] = ready;
		if (uses != 0) {
			object["uses"] = static_cast<uint64_t>(uses);
		}
	}

	void OreDeposit::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		const auto &object = json.as_object();
		oreType = boost::json::value_to<Identifier>(object.at("oreType"));
		tileID = getOre(*game).tilename;
		ready = object.contains("ready");
		if (auto *value = object.if_contains("uses")) {
			uses = value->as_uint64();
		} else {
			uses = 0;
		}
	}

	void OreDeposit::tick(const TickArgs &args) {
		TileEntity::tick(args);
		ready = true;
	}

	bool OreDeposit::onInteractNextTo(const PlayerPtr &player, Modifiers, const ItemStackPtr &, Hand) {
		if (getSide() != Side::Server)
			return false;

		const InventoryPtr inventory = player->getInventory(0);
		auto inventory_lock = inventory->uniqueLock();
		const Slot active_slot = inventory->activeSlot;
		if (ItemStackPtr active_stack = (*inventory)[active_slot]) {
			if (!ready || 0 < player->tooldown)
				return true;

			if (active_stack->hasAttribute("base:attribute/pickaxe"_id)) {
				const auto &tool = dynamic_cast<Tool &>(*active_stack->item);
				GamePtr game = player->getGame();
				const Ore &ore = getOre(*game);

				if (!inventory->add(ore.stack)) {
					player->tooldown = ore.tooldownMultiplier * tool.baseCooldown;

					if (ore.maxUses <= ++uses) {
						ready = false;
						game->enqueue(getTickFunction(), std::chrono::microseconds(static_cast<int64_t>(1e6 * ore.cooldown)));
						uses = 0;
					}

					if (active_stack->reduceDurability())
						inventory->erase(active_slot);

					inventory->notifyOwner(ore.stack);
					increaseUpdateCounter();
					queueBroadcast();
					return true;
				}
			}
		}

		return false;
	}

	void OreDeposit::render(SpriteRenderer &sprite_renderer) {
		if (!isVisible())
			return;

		RealmPtr realm = getRealm();
		Tileset &tileset = realm->getTileset();
		GamePtr game = realm->getGame();

		if (tileID == tileset.getEmpty())
			return;

		const Ore &ore = getOre(*game);
		const auto tilesize = tileset.getTileSize();
		const TileID tile_id = tileset[ready? tileID : ore.regenTilename];
		const auto texture = tileset.getTexture(*game);
		const auto x = (tile_id % (texture->width / tilesize)) * tilesize;
		const auto y = (tile_id / (texture->width / tilesize)) * tilesize;
		sprite_renderer(texture, {
			.x       = float(position.column),
			.y       = float(position.row),
			.offsetX = float(x / 2.f),
			.offsetY = float(y / 2.f),
			.sizeX   = float(tilesize),
			.sizeY   = float(tilesize),
		});
	}

	const Ore & OreDeposit::getOre(const Game &game) const {
		auto lock = oreType.sharedLock();
		return *game.registry<OreRegistry>().at(oreType);
	}

	void OreDeposit::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << oreType;
		buffer << ready;
		buffer << uses;
	}

	void OreDeposit::decode(Game &game, BasicBuffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> oreType;
		buffer >> ready;
		buffer >> uses;
	}
}
