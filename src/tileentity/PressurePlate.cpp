#include <iostream>

#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "graphics/SpriteRenderer.h"
#include "pipes/DataNetwork.h"
#include "realm/Realm.h"
#include "tileentity/PressurePlate.h"

namespace Game3 {
	PressurePlate::PressurePlate(Identifier tilename, Position position_):
		TileEntity(std::move(tilename), ID(), position_, false) {}

	PressurePlate::PressurePlate(Position position_):
		PressurePlate("base:tile/pressure_plate", position_) {}

	void PressurePlate::onOverlap(const std::shared_ptr<Entity> &entity) {
		if (getSide() != Side::Server)
			return;

		DataNetwork::broadcast(getSelf(), "Pulse");
		DataNetwork::broadcast(getSelf(), "PressurePlatePulse", entity? entity->getGID() : GlobalID(0));
	}
}
