#include "Log.h"
#include "pipes/FluidNetwork.h"

namespace Game3 {
	void FluidNetwork::tick(Tick tick_id) {
		if (!canTick(tick_id))
			return;

		PipeNetwork::tick(tick_id);
	}
}
