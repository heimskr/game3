#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "graphics/SpriteRenderer.h"
#include "pipes/DataNetwork.h"
#include "realm/Realm.h"
#include "tileentity/PressurePlate.h"

namespace Game3 {
	namespace {
		const Identifier TILE_ID_UP = "base:tile/pressure_plate";
		const Identifier TILE_ID_DOWN = "base:tile/pressure_plate_active";
	}

	PressurePlate::PressurePlate(Identifier tilename, Position position_):
		TileEntity(std::move(tilename), ID(), position_, false) {}

	PressurePlate::PressurePlate(Position position_):
		PressurePlate(TILE_ID_UP, position_) {}

	void PressurePlate::onOverlap(const EntityPtr &entity) {
		RealmPtr realm = getRealm();

		if (realm->getSide() == Side::Client) {
			setDown(true);
			return;
		}

		const bool newly_down = realm->countEntities(getPosition()) == 1;

		GlobalID gid = entity? entity->getGID() : GlobalID(0);

		if (newly_down) {
			DataNetwork::broadcast(getSelf(), "Pulse", Side::Client);
			DataNetwork::broadcast(getSelf(), "PressurePlatePulse", Side::Client, gid);
		}

		DataNetwork::broadcast(getSelf(), "PressurePlateOverlap", Side::Client, gid);
	}

	void PressurePlate::onOverlapEnd(const EntityPtr &entity) {
		if (getSide() == Side::Client) {
			setDown(isDown());
		} else {
			DataNetwork::broadcast(getSelf(), "PressurePlateOverlapEnd", Side::Client, entity? entity->getGID() : GlobalID(0), isDown());
		}
	}

	bool PressurePlate::isDown() const {
		return getRealm()->hasEntities(getPosition(), [](const EntityPtr &entity) {
			return 0.01 < entity->getOffset().z;
		});
	}

	void PressurePlate::setDown(bool new_value) {
		if (down == new_value)
			return;
		down = new_value;
		cachedTile = -1;
		tileID = down? TILE_ID_DOWN : TILE_ID_UP;
	}

	void PressurePlate::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << down;
	}

	void PressurePlate::decode(Game &game, BasicBuffer &buffer) {
		TileEntity::decode(game, buffer);
		setDown(buffer.take<bool>());
	}
}
