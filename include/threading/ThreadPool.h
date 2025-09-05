#pragma once

#include "error/ThreadPoolInactiveError.h"
#include "threading/MTQueue.h"
#include "threading/Promise.h"
#include "util/Concepts.h"

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

namespace Game3 {
	class ThreadPool {
		public:
			using Function = std::move_only_function<void(ThreadPool &, size_t)>;

			ThreadPool(size_t size_);

			~ThreadPool();

			inline bool isActive() const { return active; }
			inline auto getSize() const { return size; }

			void start();
			void join();
			/** Returns true if the pool is active and added the job, or false if the pool is inactive. */
			bool add(Function &&);

			template <std::invocable Fn>
			auto schedule(Fn &&function) {
				using Result = decltype(function());
				std::promise<Result> promise;
				std::future<Result> future = promise.get_future();

				bool added = add([promise = std::move(promise), function = std::move(function)](ThreadPool &, size_t) mutable {
					if constexpr (Returns<Fn, void>) {
						function();
						promise.set_value();
					} else {
						promise.set_value(function());
					}
				});

				if (added) {
					return future;
				}

				promise = {};
				promise.set_exception(std::make_exception_ptr(ThreadPoolInactiveError("Couldn't add job to threadpool")));
				return promise.get_future();
			}

			template <typename T>
			Ref<Promise<T>> promisify(std::future<T> &&future) {
				return Promise<T>::now([future = std::move(future)](auto resolve) mutable {
					if constexpr (std::same_as<T, void>) {
						future.wait();
						resolve();
					} else {
						resolve(future.get());
					}
				});
			}

			size_t jobCount() const;

		protected:
			const size_t size;
			MTQueue<Function> workQueue;
			std::condition_variable workCV;
			std::mutex workMutex;
			std::vector<std::thread> pool;
			std::atomic_bool active = false;
			std::atomic_bool joining = false;
			std::atomic_bool newJobReady = false;
			std::atomic_size_t jobsDone = 0;
	};
}
