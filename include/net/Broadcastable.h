#pragma once

#include "threading/Atomic.h"

namespace Game3 {
	class Broadcastable {
		public:
			Broadcastable() = default;

			void queueBroadcast(bool force = false) {
				needsBroadcast = true;
				forceBroadcast = force;
			}

		protected:
			Atomic<bool> needsBroadcast{false};
			Atomic<bool> forceBroadcast{false};
	};
}
