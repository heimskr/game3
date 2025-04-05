#include "mixin/RefCounted.h"

#include <atomic>
#include <condition_variable>
#include <expected>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

namespace Game3 {
	template <typename T, typename E>
	class Promise: public RefCounted<Promise<T, E>> {
		public:
			using Result = std::expected<T, E>;

			template <typename F>
			static Ref<Promise<T, E>> now(F &&lambda) {
				return make(std::forward<F>(lambda))->go();
			}

			template <typename F>
			static Ref<Promise<T, E>> make(F &&lambda) {
				auto *pointer = new Promise<T, E>;
				pointer->deferred = std::forward<F>(lambda);
				return pointer->getRef();
			}

			Ref<Promise<T, E>> go() {
				auto reference = this->getRef();

				std::promise<Result> promise;
				future = promise.get_future();

				thread = std::jthread([this](std::promise<Result> promise, auto &&lambda) -> void {
					lambda([this, &promise, future = future](T &&resolution) {
						promise.set_value(std::forward<T>(resolution));

						if (thenFunction) {
							thenFunction(future.get().value());
							consumed = true;
							conditionVariable.notify_all();
						}

						done();
					}, [this, &promise, future = future](E &&rejection) {
						promise.set_value(std::unexpected(std::forward<E>(rejection)));

						if (oopsFunction) {
							oopsFunction(future.get().error());
							consumed = true;
							conditionVariable.notify_all();
						}

						done();
					});
				}, std::move(promise), std::move(deferred));

				launched = true;
				return reference;
			}

			Result get() {
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
					conditionVariable.wait(lock, [this] { return consumed.load(); });
				}

				future.wait();
			}

			template <typename F>
			Promise<T, E> * then(F &&function) {
				thenFunction = std::forward<F>(function);
				return this;
			}

			template <typename F>
			Promise<T, E> * oops(F &&function) {
				oopsFunction = std::forward<F>(function);
				return this;
			}

			template <typename F>
			Promise<T, E> * finally(F &&function) {
				finallyFunction = std::forward<F>(function);
				return this;
			}

		private:
			std::shared_future<Result> future;
			std::jthread thread;
			std::atomic_bool launched = false;
			std::atomic_bool consumed = false;

			std::function<void(T)> thenFunction;
			std::function<void(E)> oopsFunction;
			std::function<void()> finallyFunction;
			std::function<void(std::function<void(T &&)> &&resolve, std::function<void(E &&)> &&reject)> deferred;

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
}
