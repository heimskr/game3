#include <iostream>

#include "Tileset.h"
#include "game/ClientGame.h"
// #include "packet/OpenEnergyLevelPacket.h"
#include "realm/Realm.h"
#include "tileentity/GeothermalGenerator.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	GeothermalGenerator::GeothermalGenerator(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true) {}

	GeothermalGenerator::GeothermalGenerator(Position position_):
		GeothermalGenerator("base:tile/geothermal_generator"_id, position_) {}

	EnergyAmount GeothermalGenerator::getEnergyCapacity() {
		return 64'000;
	}

	void GeothermalGenerator::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
	}

	bool GeothermalGenerator::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers) {
		auto &realm = *getRealm();

		if (modifiers.onlyAlt()) {
			realm.queueDestruction(shared_from_this());
			player->give(ItemStack(realm.getGame(), "base:item/geothermal_generator"_id));
			return true;
		}

		// TODO!
		// player->send(OpenEnergyLevelPacket(getGID()));

		std::shared_lock lock{energyMutex};
		INFO("Energy: " << energyAmount);
		return false;
	}

	void GeothermalGenerator::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
	}

	void GeothermalGenerator::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
	}

	void GeothermalGenerator::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
	}
}
