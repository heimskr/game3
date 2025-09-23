#pragma once

#include <functional>
#include <memory>
#include <mutex>

namespace Game3 {
	template <typename Signature>
	class SharedFunction;

	template <typename Result, typename... Args>
	class SharedFunction<Result(Args...)> {
		public:
			using Function = std::move_only_function<Result(Args...)>;

			SharedFunction() = default;

			template <typename F>
			requires (!std::same_as<std::remove_cvref_t<F>, SharedFunction<Result(Args...)>>)
			SharedFunction(F &&function):
				function(std::make_shared<Function>(std::forward<F>(function))) {}

			SharedFunction(const SharedFunction<Result(Args...)> &other) {
				std::unique_lock lock{other.mutex};
				function = other.function;
			}

			SharedFunction(SharedFunction<Result(Args...)> &&other) {
				std::unique_lock lock{other.mutex};
				function = std::move(other.function);
			}

			SharedFunction<Result(Args...)> & operator=(const SharedFunction<Result(Args...)> &other) {
				if (this != &other) {
					std::scoped_lock lock{mutex, other.mutex};
					function = other.function;
				}

				return *this;
			}

			SharedFunction<Result(Args...)> & operator=(SharedFunction<Result(Args...)> &&other) {
				if (this != &other) {
					std::scoped_lock lock{mutex, other.mutex};
					function = std::move(other.function);
				}

				return *this;
			}

			explicit operator bool() const {
				std::unique_lock lock{mutex};
				return static_cast<bool>(*function);
			}

			template <typename... CallArgs>
			decltype(auto) operator()(this auto &&self, CallArgs &&...args) {
				std::unique_lock lock{self.mutex};
				return (*self.function)(std::forward<CallArgs>(args)...);
			}

			decltype(auto) operator*(this auto &&self) {
				return *self.function;
			}

		private:
			std::shared_ptr<Function> function;
			mutable std::mutex mutex;
	};

	template <typename Result>
	class SharedFunction<Result(void)> {
		public:
			using Function = std::move_only_function<Result()>;

			SharedFunction() = default;

			template <typename F>
			requires (!std::same_as<std::remove_cvref_t<F>, SharedFunction<Result(void)>>)
			SharedFunction(F &&function):
				function(std::make_shared<Function>(std::forward<F>(function))) {}

			SharedFunction(const SharedFunction<Result(void)> &other) {
				std::unique_lock lock{other.mutex};
				function = other.function;
			}

			SharedFunction(SharedFunction<Result(void)> &&other) {
				std::unique_lock lock{other.mutex};
				function = std::move(other.function);
			}

			SharedFunction<Result(void)> & operator=(const SharedFunction<Result(void)> &other) {
				if (this != &other) {
					std::scoped_lock lock{mutex, other.mutex};
					function = other.function;
				}

				return *this;
			}

			SharedFunction<Result(void)> & operator=(SharedFunction<Result(void)> &&other) {
				if (this != &other) {
					std::scoped_lock lock{mutex, other.mutex};
					function = std::move(other.function);
				}

				return *this;
			}

			explicit operator bool() const {
				std::unique_lock lock{mutex};
				return static_cast<bool>(*function);
			}

			template <typename... CallArgs>
			decltype(auto) operator()(this auto &&self, CallArgs &&...args) {
				std::unique_lock lock{self.mutex};
				return (*self.function)(std::forward<CallArgs>(args)...);
			}

			decltype(auto) operator*(this auto &&self) {
				return *self.function;
			}

		private:
			std::shared_ptr<Function> function;
			mutable std::mutex mutex;
	};
}
