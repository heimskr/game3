#pragma once

#include "threading/Atomic.h"

namespace Game3 {
	class Broadcastable {
		public:
			Broadcastable() = default;

			virtual ~Broadcastable() = default;

			void queueBroadcast(bool force = false);

		protected:
			Atomic<bool> needsBroadcast{false};
			Atomic<bool> forceBroadcast{false};
	};
}
