#pragma once

#include "threading/Promise.h"
#include "util/Concepts.h"

#include <chrono>
#include <thread>

namespace Game3 {
	template <Duration D>
	PromiseRef<void> sleepFor(D duration) {
		return Promise<void>::now([duration](auto resolve) {
			std::this_thread::sleep_for(duration);
			resolve();
		});
	}

	PromiseRef<void> sleepFor(uint64_t milliseconds);

	template <typename T, Duration D>
	PromiseRef<T> delayFor(T value, D duration) {
		return Promise<T>::now([value = std::move(value), duration](auto resolve) mutable {
			std::this_thread::sleep_for(duration);
			resolve(std::move(value));
		});
	}

	template <typename T>
	PromiseRef<T> delayFor(T value, uint64_t milliseconds) {
		return delayFor<T>(std::move(value), std::chrono::milliseconds(milliseconds));
	}
}
