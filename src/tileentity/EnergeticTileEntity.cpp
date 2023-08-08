#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
// #include "packet/OpenEnergyLevelPacket.h"
#include "packet/SetTileEntityEnergyPacket.h"
#include "realm/Realm.h"
#include "tileentity/EnergeticTileEntity.h"

namespace Game3 {
	EnergeticTileEntity::EnergeticTileEntity(EnergyAmount capacity, EnergyAmount energy):
		HasEnergy(capacity, energy) {}

	bool EnergeticTileEntity::canInsertEnergy(EnergyAmount amount, Direction) {
		assert(energyContainer);
		std::shared_lock lock{energyContainer->mutex};
		return energyContainer->canInsert(amount);
	}

	EnergyAmount EnergeticTileEntity::addEnergy(EnergyAmount amount, Direction) {
		return addEnergy(amount);
	}

	EnergyAmount EnergeticTileEntity::extractEnergy(Direction, bool remove, EnergyAmount max_amount) {
		assert(energyContainer);
		std::unique_lock lock{energyContainer->mutex};
		const EnergyAmount to_remove = std::min(max_amount, energyContainer->energy);
		if (0 < remove)
			energyContainer->energy -= to_remove;
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
		player->queueForMove([this, self = shared_from_this()](const std::shared_ptr<Entity> &entity) {
			removeObserver(std::static_pointer_cast<Player>(entity));
			return true;
		});
	}

	void EnergeticTileEntity::toJSON(nlohmann::json &json) const {
		std::shared_lock lock{energyContainer->mutex};
		json["energy"] = energyContainer->energy;
	}

	void EnergeticTileEntity::absorbJSON(Game &, const nlohmann::json &json) {
		const EnergyAmount amount = json.at("energy");
		std::unique_lock lock{energyContainer->mutex};
		energyContainer->energy = amount;
	}

	void EnergeticTileEntity::encode(Game &, Buffer &buffer) {
		HasEnergy::encode(buffer);
	}

	void EnergeticTileEntity::decode(Game &, Buffer &buffer) {
		EnergyAmount old_amount{};
		{
			std::shared_lock lock{energyContainer->mutex};
			old_amount = energyContainer->energy;
		}
		HasEnergy::decode(buffer);
		bool updated = false;
		{
			std::shared_lock lock{energyContainer->mutex};
			updated = old_amount != energyContainer->energy;
		}
		if (updated)
			energyUpdated();
	}

	void EnergeticTileEntity::broadcast() {
		if (forceBroadcast)
			TileEntity::broadcast();
		else
			broadcast(makeEnergyPacket());
	}

	SetTileEntityEnergyPacket EnergeticTileEntity::makeEnergyPacket() const {
		const GlobalID gid = getGID();
		EnergyAmount energy{};
		{
			auto lock = energyContainer->sharedLock();
			energy = energyContainer->energy;
		}
		return {gid, energy};
	}

	void EnergeticTileEntity::broadcast(const SetTileEntityEnergyPacket &packet) {
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
