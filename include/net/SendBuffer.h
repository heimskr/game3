#pragma once

#include <atomic>
#include <cassert>
#include <mutex>
#include <shared_mutex>
#include <vector>

namespace Game3 {
	struct SendBuffer {
		std::shared_mutex mutex;
		std::atomic_size_t depth = 0;
		std::vector<char> bytes;
		SendBuffer() = default;

		inline auto sharedLock() { return std::shared_lock(mutex); }
		inline auto uniqueLock() { return std::unique_lock(mutex); }
		inline SendBuffer & operator++() { ++depth; return *this; }
		inline SendBuffer & operator--() { assert(0 <= --depth); return *this; }
		inline bool active() const { return 0 < depth; }
	};
}
