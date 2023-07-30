#include "game/ClientGame.h"
// #include "packet/OpenEnergyLevelPacket.h"
#include "packet/TileEntityPacket.h"
#include "realm/Realm.h"
#include "tileentity/EnergeticTileEntity.h"

namespace Game3 {
	EnergeticTileEntity::EnergeticTileEntity(EnergyAmount amount):
		HasEnergy(amount) {}

	bool EnergeticTileEntity::canInsertEnergy(EnergyAmount amount, Direction) {
		std::shared_lock lock{energyMutex};
		return energyAmount + amount <= getEnergyCapacity();
	}

	EnergyAmount EnergeticTileEntity::addEnergy(EnergyAmount amount, Direction) {
		return addEnergy(amount);
	}

	EnergyAmount EnergeticTileEntity::extractEnergy(Direction, bool remove, EnergyAmount max_amount) {
		std::unique_lock lock{energyMutex};

		const EnergyAmount to_remove = std::min(max_amount, energyAmount);

		if (remove)
			energyAmount -= to_remove;

		return to_remove;
	}

	EnergyAmount EnergeticTileEntity::extractEnergy(Direction direction, bool remove) {
		return extractEnergy(direction, remove, std::numeric_limits<EnergyAmount>::max());
	}

	void EnergeticTileEntity::energyUpdated() {
		auto realm = weakRealm.lock();
		if (!realm)
			return;

		if (realm->getSide() == Side::Server) {
			increaseUpdateCounter();
			queueBroadcast();
		} else {
			getRealm()->getGame().toClient().signal_energy_update().emit(std::dynamic_pointer_cast<HasEnergy>(shared_from_this()));
		}
	}

	void EnergeticTileEntity::addObserver(const std::shared_ptr<Player> &player) {
		Observable::addObserver(player);
		player->send(TileEntityPacket(shared_from_this()));
		// TODO!
		// player->send(OpenEnergyLevelPacket(getGID()));
		player->queueForMove([this](const std::shared_ptr<Entity> &entity) {
			removeObserver(std::static_pointer_cast<Player>(entity));
			return true;
		});
	}

	void EnergeticTileEntity::toJSON(nlohmann::json &json) const {
		std::shared_lock lock{const_cast<std::shared_mutex &>(energyMutex)};
		json["energy"] = energyAmount;
	}

	void EnergeticTileEntity::absorbJSON(Game &, const nlohmann::json &json) {
		const EnergyAmount amount = json.at("energy");
		std::unique_lock lock{const_cast<std::shared_mutex &>(energyMutex)};
		energyAmount = amount;
	}

	void EnergeticTileEntity::encode(Game &, Buffer &buffer) {
		HasEnergy::encode(buffer);
	}

	void EnergeticTileEntity::decode(Game &, Buffer &buffer) {
		EnergyAmount old_amount{};
		{
			std::shared_lock lock{energyMutex};
			old_amount = energyAmount;
		}
		HasEnergy::decode(buffer);
		bool updated = false;
		{
			std::shared_lock lock{energyMutex};
			updated = old_amount != energyAmount;
		}
		if (updated)
			energyUpdated();
	}

	void EnergeticTileEntity::broadcast() {
		if (forceBroadcast)
			TileEntity::broadcast();
		else
			broadcast(TileEntityPacket(shared_from_this()));
	}

	void EnergeticTileEntity::broadcast(const TileEntityPacket &packet) {
		assert(getSide() == Side::Server);
		auto lock = observers.uniqueLock();

		std::erase_if(observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				player->send(packet);
				return false;
			}

			return false;
		});
	}
}
