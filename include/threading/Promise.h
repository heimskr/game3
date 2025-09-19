#pragma once

#include "mixin/RefCounted.h"
#include "util/Concepts.h"

#include <atomic>
#include <condition_variable>
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

namespace Game3 {
	template <template <typename...> typename F, typename U>
	struct VoidFunction {
		using Type = F<void(U)>;
	};

	template <template <typename...> typename F>
	struct VoidFunction<F, void> {
		using Type = F<void()>;
	};

	template <typename T>
	class Promise: public RefCounted<Promise<T>> {
		public:
			template <typename F>
			static Ref<Promise<T>> now(F &&lambda) {
				return make(std::move(lambda))->go();
			}

			template <typename F>
			static Ref<Promise<T>> make(F &&lambda) {
				auto *pointer = new Promise<T>;
				pointer->deferred = std::move(lambda);
				return pointer->getRef();
			}

			static Ref<Promise<void>> resolved() {
				static_assert(std::same_as<T, void>);
				return Promise<void>::now([](auto &&resolve, auto &&) {
					resolve();
				});
			}

			template <typename U>
			static Ref<Promise<T>> resolved(U &&value) {
				static_assert(!std::same_as<T, void>);
				return Promise<T>::now([value = std::forward<U>(value)](auto &&resolve, auto &&) mutable {
					resolve(std::forward<U>(value));
				});
			}

			Ref<Promise<T>> go() {
				auto reference = this->getRef();

				if (launched) {
					return reference;
				}

				future = promise.get_future();

				thread = std::jthread([this](auto &&lambda) {
					try {
						auto reject = Function<const std::exception &>([this](const std::exception &error) mutable {
							try {
								throw error;
							} catch (...) {
								throwException(std::current_exception());
							}
						});

						if constexpr (std::same_as<T, void>) {
							auto resolve = [this] mutable {
								promise.set_value();

								if (thenFunction) {
									thenFunction();
									consumed = true;
								}

								finished = true;
								conditionVariable.notify_all();
								done();
							};

							lambda(std::move(resolve), std::move(reject));
						} else {
							auto resolve = [this](T &&resolution) mutable {
								promise.set_value(std::forward<T>(resolution));

								if (thenFunction) {
									thenFunction(future.get());
									consumed = true;
								}

								finished = true;
								conditionVariable.notify_all();
								done();
							};

							lambda(std::move(resolve), std::move(reject));
						}
					} catch (...) {
						throwException(std::current_exception());
					}
				}, std::move(deferred));

				thread.detach();

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
				if (consumed || finished) {
					return;
				}

				auto reference = this->getRef();

				if (!launched) {
					go();
				}

				std::unique_lock lock(mutex);
				conditionVariable.wait(lock, [this] { return consumed || rejected || finished; });

				if (!rejected && !finished) {
					future.wait();
				}
			}

			template <typename F>
			Ref<Promise<T>> then(F &&function) {
				thenFunction = std::move(function);
				return this->getRef();
			}

			template <typename FS, typename FE>
			Ref<Promise<T>> then(FS &&on_resolve, FE &&on_reject) {
				thenFunction = std::move(on_resolve);
				return oops(std::move(on_reject));
			}

			template <std::invocable<const std::exception &> F>
			Ref<Promise<T>> oops(F &&function) {
				oopsFunction = [function = std::move(function)](std::exception_ptr ptr) mutable {
					try {
						std::rethrow_exception(std::move(ptr));
					} catch (const std::exception &error) {
						function(error);
					}
				};
				return this->getRef();
			}

			template <std::invocable<std::exception_ptr> F>
			Ref<Promise<T>> oops(F &&function) {
				oopsFunction = std::move(function);
				return this->getRef();
			}

			template <typename F>
			requires Returns<F, void, std::exception_ptr>
			Ref<Promise<T>> finally(F &&function) {
				finallyFunction = std::move(function);
				return this->getRef();
			}

			template <typename F>
			requires Returns<F, void>
			Ref<Promise<T>> finally(F &&function) {
				finallyFunction = [function = std::move(function)](std::exception_ptr) mutable {
					function();
				};
				return this->getRef();
			}

		private:
			std::promise<T> promise;
			std::shared_future<T> future;
			std::jthread thread;
			std::atomic_bool launched = false;
			std::atomic_bool consumed = false;
			std::atomic_bool rejected = false;
			std::atomic_bool finished = false;

			template <typename U>
			using MoveOnlyFunction = VoidFunction<std::move_only_function, U>::Type;

			template <typename U>
			using Function = VoidFunction<std::function, U>::Type;

			MoveOnlyFunction<T> thenFunction;
			std::move_only_function<void(std::exception_ptr)> oopsFunction;
			std::move_only_function<void(std::exception_ptr)> finallyFunction; // the std::exception_ptr argument is null if no exception was thrown
			std::move_only_function<void(Function<T> &&resolve, Function<const std::exception &> &&reject)> deferred;

			std::condition_variable conditionVariable;
			std::mutex mutex;

			Promise() = default;

			void done() {
				this->deref();
			}

			void throwException(const std::exception_ptr &exception) {
				if (oopsFunction) {
					oopsFunction(exception);
					consumed = true;
					rejected = true;
					finished = true;
					conditionVariable.notify_all();
					if (finallyFunction) {
						finallyFunction(exception);
					}
					done();
				} else if (finallyFunction) {
					consumed = true;
					rejected = true;
					finished = true;
					conditionVariable.notify_all();
					finallyFunction(exception);
					done();
				} else {
					done();
					std::rethrow_exception(exception);
				}
			}
	};

	template <typename T>
	using PromiseRef = Ref<Promise<T>>;
}
