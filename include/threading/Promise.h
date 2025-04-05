#include "mixin/RefCounted.h"

#include <atomic>
#include <expected>
#include <functional>
#include <future>
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
						const bool was_consumed = thenFunction && consumed.exchange(true);

						promise.set_value(std::forward<T>(resolution));

						if (thenFunction && !was_consumed) {
							thenFunction(future.get().value());
						}

						done();
					}, [this, &promise, future = future](E &&rejection) {
						const bool was_consumed = oopsFunction && consumed.exchange(true);

						promise.set_value(std::unexpected(std::forward<E>(rejection)));

						if (oopsFunction && !was_consumed) {
							oopsFunction(future.get().error());
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

				if (consumed.exchange(true)) {
					throw std::runtime_error("Cannot get from a consumed Promise");
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
			std::atomic_bool undeleted = false;
			std::atomic_bool consumed = false;

			std::function<void(T)> thenFunction;
			std::function<void(E)> oopsFunction;
			std::function<void()> finallyFunction;
			std::function<void(std::function<void(T &&)> &&resolve, std::function<void(E &&)> &&reject)> deferred;

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
