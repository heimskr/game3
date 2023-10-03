#include "Log.h"
#include "game/Agent.h"
#include "net/Broadcastable.h"

namespace Game3 {
	void Broadcastable::queueBroadcast(bool force) {
		INFO(dynamic_cast<Agent &>(*this).getName() << "->queueBroadcast(" << force << ')');
		needsBroadcast = true;
		forceBroadcast = force;
	}
}
