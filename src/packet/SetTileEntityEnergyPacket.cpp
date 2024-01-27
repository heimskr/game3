#include "Log.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "packet/SetTileEntityEnergyPacket.h"
#include "tileentity/EnergeticTileEntity.h"

namespace Game3 {
	void SetTileEntityEnergyPacket::handle(ClientGame &game) {
		AgentPtr agent = game.getAgent(agentGID);
		if (!agent) {
			ERROR_("Couldn't find agent " << agentGID << " in SetTileEntityEnergyPacket handler");
			return;
		}

		auto energetic = std::dynamic_pointer_cast<EnergeticTileEntity>(agent);
		if (!energetic) {
			ERROR_("Agent " << agentGID << " isn't an instance of EnergeticTileEntity");
			return;
		}

		auto lock = energetic->energyContainer->uniqueLock();
		energetic->setEnergy(newEnergy);
	}
}
