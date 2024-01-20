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
	class HasTickQueue {
		public:
			using Function = std::function<void(Tick)>;

			void tick();

			void tick(double delta, double frequency);

			template <chrono_duration D>
			void enqueueServer(Function function, D delay) {
				enqueueAt(std::move(function), currentTick + getDelayTicks(delay, SERVER_TICK_FREQUENCY));
			}

			template <chrono_duration D>
			void enqueueClient(Function function, D delay) {
				enqueueAt(std::move(function), currentTick + getDelayTicks(delay, CLIENT_TICK_FREQUENCY));
			}

			template <chrono_duration D>
			void enqueue(Function function, D delay, bool is_server) {
				if (is_server)
					enqueueServer(std::move(function), delay);
				else
					enqueueClient(std::move(function), delay);
			}

		private:
			Atomic<Tick> currentTick = 0;
			Lockable<std::multimap<Tick, Function>> tickQueue;

			void enqueueAt(Function, Tick);
			void dequeueAll();

			template <chrono_duration D>
			constexpr static Tick getDelayTicks(D delay, int64_t frequency) {
				return std::chrono::duration_cast<std::chrono::seconds>(delay).count() * frequency;
			}
	};
}
