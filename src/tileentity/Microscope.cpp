#include <iostream>

#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/ServerInventory.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "realm/Realm.h"
#include "tileentity/Microscope.h"
#include "ui/module/MicroscopeModule.h"

namespace Game3 {
	Microscope::Microscope(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true) {}

	Microscope::Microscope(Position position_):
		Microscope("base:tile/microscope"_id, position_) {}

	void Microscope::init(Game &game) {
		TileEntity::init(game);
		HasInventory::setInventory(Inventory::create(shared_from_this(), 1), 0);
	}

	void Microscope::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
	}

	bool Microscope::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		if (modifiers.onlyAlt()) {
			RealmPtr realm = getRealm();
			getInventory(0)->iterate([&](const ItemStackPtr &stack, Slot) {
				stack->spawn(getPlace());
				return false;
			});
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/microscope"_id));
			return true;
		}

		player->send(OpenModuleForAgentPacket(MicroscopeModule::ID(), getGID()));
		addObserver(player, true);

		return false;
	}

	void Microscope::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
	}

	void Microscope::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
	}

	void Microscope::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
	}

	void Microscope::broadcast(bool force) {
		InventoriedTileEntity::broadcast(force);
	}

	GamePtr Microscope::getGame() const {
		return TileEntity::getGame();
	}
}
