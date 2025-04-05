#pragma once

#include "mixin/RefCounted.h"

#include <atomic>
#include <condition_variable>
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

namespace Game3 {
	template <typename T>
	class Promise: public RefCounted<Promise<T>> {
		public:
			template <typename F>
			static Ref<Promise<T>> now(F &&lambda) {
				return make(std::forward<F>(lambda))->go();
			}

			template <typename F>
			static Ref<Promise<T>> make(F &&lambda) {
				auto *pointer = new Promise<T>;
				pointer->deferred = std::forward<F>(lambda);
				return pointer->getRef();
			}

			Ref<Promise<T>> go() {
				auto reference = this->getRef();

				std::promise<T> promise;
				future = promise.get_future();

				thread = std::jthread([this](std::promise<T> promise, auto &&lambda) -> void {
					try {
						if constexpr (typeid(T) == typeid(void)) {
							lambda([this, &promise, future = future]() {
								promise.set_value();

								if (thenFunction) {
									thenFunction();
									consumed = true;
									conditionVariable.notify_all();
								}

								done();
							});
						} else {
							lambda([this, &promise, future = future](T &&resolution) {
								promise.set_value(std::forward<T>(resolution));

								if (thenFunction) {
									thenFunction(future.get());
									consumed = true;
									conditionVariable.notify_all();
								}

								done();
							});
						}
					} catch (...) {
						if (oopsFunction) {
							oopsFunction(std::current_exception());
							consumed = true;
							rejected = true;
							conditionVariable.notify_all();
							done();
						} else {
							done();
							throw;
						}
					}
				}, std::move(promise), std::move(deferred));

				launched = true;
				return reference;
			}

			T get() {
				auto reference = this->getRef();

				if (!launched) {
					go();
				}

				if (thenFunction || oopsFunction) {
					throw std::runtime_error("Cannot get from a Promise with a then or catch function");
				}

				future.wait();
				return future.get();
			}

			void wait() {
				if (consumed) {
					return;
				}

				auto reference = this->getRef();

				if (!launched) {
					go();
				}

				if (thenFunction || oopsFunction) {
					std::unique_lock lock(mutex);
					conditionVariable.wait(lock, [this] { return consumed.load() || rejected.load(); });
				}

				if (!rejected) {
					future.wait();
				}
			}

			template <typename F>
			Ref<Promise<T>> then(F &&function) {
				thenFunction = std::forward<F>(function);
				return this->getRef();
			}

			template <typename F>
			Ref<Promise<T>> oops(F &&function) {
				oopsFunction = std::forward<F>(function);
				return this->getRef();
			}

			template <typename F>
			Ref<Promise<T>> finally(F &&function) {
				finallyFunction = std::forward<F>(function);
				return this->getRef();
			}

		private:
			std::shared_future<T> future;
			std::jthread thread;
			std::atomic_bool launched = false;
			std::atomic_bool consumed = false;
			std::atomic_bool rejected = false;

			template <typename U>
			struct FunctionStruct {
				using Type = std::function<void(U)>;
			};

			template <>
			struct FunctionStruct<void> {
				using Type = std::function<void()>;
			};

			template <typename U>
			using Function = FunctionStruct<U>::Type;

			Function<T> thenFunction;
			std::function<void(std::exception_ptr)> oopsFunction;
			std::function<void()> finallyFunction;
			std::function<void(Function<T> &&resolve)> deferred;

			std::condition_variable conditionVariable;
			std::mutex mutex;

			Promise() = default;

			void done() {
				if (finallyFunction) {
					finallyFunction();
				}

				if (launched) {
					thread.detach();
				}

				this->deref();
			}
	};

	template <typename T>
	using PromiseRef = Ref<Promise<T>>;
}
