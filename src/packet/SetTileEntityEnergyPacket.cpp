#include "Log.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "packet/SetTileEntityEnergyPacket.h"
#include "tileentity/EnergeticTileEntity.h"

namespace Game3 {
	void SetTileEntityEnergyPacket::handle(const ClientGamePtr &game) {
		AgentPtr agent = game->getAgent(agentGID);
		if (!agent) {
			ERROR("Couldn't find agent {} in SetTileEntityEnergyPacket handler", agentGID);
			return;
		}

		auto energetic = std::dynamic_pointer_cast<EnergeticTileEntity>(agent);
		if (!energetic) {
			ERROR("Agent {} isn't an instance of EnergeticTileEntity", agentGID);
			return;
		}

		auto lock = energetic->energyContainer->uniqueLock();
		energetic->setEnergy(newEnergy);
	}
}
