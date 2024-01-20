#pragma once

#include "game/SimulationOptions.h"
#include "threading/Atomic.h"
#include "threading/Lockable.h"
#include "types/Types.h"
#include "util/Concepts.h"

#include <chrono>
#include <functional>
#include <map>

namespace Game3 {
	template <typename... FunctionArgs>
	class HasTickQueue {
		public:
			virtual double getFrequency() const = 0;

			template <typename... Args>
			void tick(Args &&...args) {
				++currentTick;
				dequeueAll(std::forward<Args>(args)...);
			}

			template <Numeric Delta = double, typename... Args>
			void tick(Delta delta, Args &&...args) {
				currentTick += Tick(std::max(1.0, delta * getFrequency()));
				dequeueAll(std::forward<Args>(args)...);
			}

			inline Tick getCurrentTick() const {
				return currentTick;
			}

			Tick enqueue(std::function<void(FunctionArgs...)> function) {
				tickQueue.emplace(currentTick + 1, std::move(function));
				return currentTick + 1;
			}

			template <Duration D>
			Tick enqueue(std::function<void(FunctionArgs...)> function, D delay) {
				const Tick tick = currentTick + getDelayTicks(delay);
				tickQueue.emplace(tick, std::move(function));
				return tick;
			}

			template <typename Function, Duration D>
			requires (!std::is_same_v<Function, std::function<void(FunctionArgs...)>>)
			Tick enqueue(Function function, D delay) {
				auto lock = tickQueue.uniqueLock();
				const Tick tick = currentTick + getDelayTicks(delay);
				tickQueue.emplace(tick, [function = std::move(function)](FunctionArgs &&...args) {
					function(std::forward<FunctionArgs>(args)...);
				});
				return tick;
			}

			template <typename Function>
			requires (!std::is_same_v<Function, std::function<void(FunctionArgs...)>>)
			Tick enqueue(Function function) {
				auto lock = tickQueue.uniqueLock();
				tickQueue.emplace(currentTick + 1, [function = std::move(function)](FunctionArgs &&...args) {
					function(std::forward<FunctionArgs>(args)...);
				});
				return currentTick + 1;
			}

		private:
			Atomic<Tick> currentTick = 0;
			Lockable<std::multimap<Tick, std::function<void(FunctionArgs...)>>> tickQueue;

			template <typename... Args>
			void dequeueAll(Args &&...args) {
				auto lock = tickQueue.uniqueLock();

				// Call and remove all queued functions that should execute now or should've been executed by now.
				for (auto iter = tickQueue.begin(); iter != tickQueue.end() && iter->first <= currentTick;) {
					iter->second(std::forward<Args>(args)...);
					iter = tickQueue.erase(iter);
				}
			}

			template <Duration D>
			Tick getDelayTicks(D delay) {
				return Tick(std::max(1.0, std::chrono::duration_cast<std::chrono::microseconds>(delay).count() * getFrequency() / 1e6));
			}
	};
}
