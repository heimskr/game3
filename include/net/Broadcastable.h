#pragma once

#include <atomic>

namespace Game3 {
	class Broadcastable {
		public:
			Broadcastable() = default;

			Broadcastable(const Broadcastable &other):
				needsBroadcast(other.needsBroadcast.load()) {}

			Broadcastable(Broadcastable &&other):
				needsBroadcast(other.needsBroadcast.load()) {}

			Broadcastable & operator=(const Broadcastable &other) {
				needsBroadcast = other.needsBroadcast.load();
				return *this;
			}

			Broadcastable & operator=(Broadcastable &&other) {
				needsBroadcast = other.needsBroadcast.load();
				return *this;
			}

			void queueBroadcast(bool force = false) {
				needsBroadcast = true;
				forceBroadcast = force;
			}

		protected:
			std::atomic_bool needsBroadcast{false};
			std::atomic_bool forceBroadcast{false};
	};
}
