#pragma once

#include "threading/Atomic.h"
#include "util/BufferUtil.h"

namespace Game3 {
	template <typename T = float>
	struct TimeAccumulator {
		Atomic<T> accumulatedTime = 0;

		TimeAccumulator(T accumulated_time = 0):
			accumulatedTime(accumulated_time) {}

		template <typename D>
		void accumulateTime(D delta) {
			accumulatedTime += delta;
		}

		template <typename D, typename P>
		bool accumulateTime(D delta, P period) {
			return period <= (accumulatedTime += delta);
		}

		void encode(Buffer &buffer) {
			addToBuffer<T>(accumulatedTime.load());
		}

		void decode(BasicBuffer &buffer) {
			accumulatedTime = getFromBuffer<T>(buffer);
		}
	};
}
