#include "Log.h"
#include "pipes/EnergyNetwork.h"

namespace Game3 {
	void EnergyNetwork::tick(Tick tick_id) {
		if (!canTick(tick_id))
			return;

		PipeNetwork::tick(tick_id);
	}
}
