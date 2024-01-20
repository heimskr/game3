#pragma once

#include "threading/Atomic.h"

namespace Game3 {
	class Broadcastable {
		public:
			Broadcastable() = default;

			virtual ~Broadcastable() = default;

			virtual void queueBroadcast(bool force);

			void queueBroadcast() { queueBroadcast(false); }

		protected:
			Atomic<bool> needsBroadcast{false};
			Atomic<bool> forceBroadcast{false};
	};
}
