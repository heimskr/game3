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
	template <Numeric Delta = double, typename... FunctionArgs>
	class HasTickQueue {
		public:
			virtual double getFrequency() const = 0;

			template <typename... Args>
			void tick(Args &&...args) {
				++currentTick;
				dequeueAll(std::forward<Args>(args)...);
			}

			template <typename... Args>
			void tick(Delta delta, Args &&...args) {
				currentTick += Tick(std::max(1.0, delta * getFrequency()));
				dequeueAll(std::forward<Args>(args)...);
			}

			inline Tick getCurrentTick() const {
				return currentTick;
			}

			void enqueue(std::function<void(FunctionArgs...)> function) {
				tickQueue.emplace(currentTick + 1, std::move(function));
			}

			template <Duration D>
			void enqueue(std::function<void(FunctionArgs...)> function, D delay) {
				tickQueue.emplace(currentTick + getDelayTicks(delay), std::move(function));
			}

			template <typename Function, Duration D>
			requires (!std::is_same_v<Function, std::function<void(FunctionArgs...)>>)
			void enqueue(Function function, D delay) {
				auto lock = tickQueue.uniqueLock();
				tickQueue.emplace(currentTick + getDelayTicks(delay), [function = std::move(function)](FunctionArgs &&...args) {
					function(std::forward<FunctionArgs>(args)...);
				});
			}

			template <typename Function>
			requires (!std::is_same_v<Function, std::function<void(FunctionArgs...)>>)
			void enqueue(Function function) {
				auto lock = tickQueue.uniqueLock();
				tickQueue.emplace(currentTick + 1, [function = std::move(function)](FunctionArgs &&...args) {
					function(std::forward<FunctionArgs>(args)...);
				});
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
				return Tick(std::chrono::duration_cast<std::chrono::seconds>(delay).count() * getFrequency());
			}
	};
}
