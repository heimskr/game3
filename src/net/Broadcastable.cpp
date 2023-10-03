#include "game/Agent.h"
#include "net/Broadcastable.h"

namespace Game3 {
	void Broadcastable::queueBroadcast(bool force) {
		needsBroadcast = true;
		forceBroadcast = force;
	}
}
