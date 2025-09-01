#include "lib/JSON.h"
#include "tileentity/DirectedTileEntity.h"

namespace Game3 {
	DirectedTileEntity::DirectedTileEntity(Direction direction):
		tileDirection(direction) {}

	void DirectedTileEntity::setDirection(Direction new_direction) {
		tileDirection = new_direction;

		switch (tileDirection) {
			case Direction::Up:    tileID = Identifier(getDirectedTileBase() + 'n'); break;
			case Direction::Right: tileID = Identifier(getDirectedTileBase() + 'e'); break;
			case Direction::Down:  tileID = Identifier(getDirectedTileBase() + 's'); break;
			case Direction::Left:  tileID = Identifier(getDirectedTileBase() + 'w'); break;
			default:
				tileID = "base:tile/missing"_id;
		}

		cachedTile = -1;
	}

	void DirectedTileEntity::rotateClockwise() {
		setDirection(Game3::rotateClockwise(getDirection()));
		increaseUpdateCounter();
		queueBroadcast(true);
	}

	void DirectedTileEntity::rotateCounterClockwise() {
		setDirection(Game3::rotateCounterClockwise(getDirection()));
		increaseUpdateCounter();
		queueBroadcast(true);
	}

	void DirectedTileEntity::toJSON(boost::json::value &json) const {
		ensureObject(json)["direction"] = boost::json::value_from(tileDirection);
	}

	void DirectedTileEntity::absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &json) {
		setDirection(boost::json::value_to<Direction>(json.at("direction")));
	}

	void DirectedTileEntity::encode(Game &, Buffer &buffer) {
		buffer << getDirection();
	}

	void DirectedTileEntity::decode(Game &, BasicBuffer &buffer) {
		setDirection(buffer.take<Direction>());
	}
}
