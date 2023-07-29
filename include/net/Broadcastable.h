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

			void queueBroadcast() { needsBroadcast = true; }

		protected:
			std::atomic_bool needsBroadcast{false};
	};
}
