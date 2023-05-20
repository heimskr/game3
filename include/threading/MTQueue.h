#pragma once

#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>

namespace Game3 {
	/** A threadsafe but slow queue. */
	template <typename T, typename C = std::deque<T>>
	class MTQueue: private std::queue<T, C> {
		private:
			std::shared_mutex mutex;

		public:
			MTQueue() = default;
			using std::queue<T, C>::queue;

			inline T & front() {
				std::shared_lock lock(mutex);
				return std::queue<T, C>::front();
			}

			inline T & back() {
				std::shared_lock lock(mutex);
				return std::queue<T, C>::back();
			}

			inline void pop() {
				std::unique_lock lock(mutex);
				std::queue<T, C>::pop();
			}

			inline T take() {
				std::unique_lock lock(mutex);
				T out = std::move(std::queue<T, C>::back());
				std::queue<T, C>::pop();
				return std::move(out);
			}

			inline std::optional<T> tryTake() {
				std::unique_lock lock(mutex);
				if (std::queue<T, C>::empty())
					return std::nullopt;
				auto out = std::make_optional(std::move(std::queue<T, C>::back()));
				std::queue<T, C>::pop();
				return std::move(out);
			}

			inline void push(T &&value) {
				std::unique_lock lock(mutex);
				std::queue<T, C>::push(std::move(value));
			}

			inline void push(const T &value) {
				std::unique_lock lock(mutex);
				std::queue<T, C>::push(value);
			}

			inline bool empty() {
				std::shared_lock lock(mutex);
				return std::queue<T, C>::empty();
			}

			inline void clear() {
				std::unique_lock lock(mutex);
				this->c.clear();
			}

			template <typename... Args>
			inline auto emplace(Args &&...args) {
				std::unique_lock lock(mutex);
				return std::queue<T, C>::emplace(std::forward<Args>(args)...);
			}

			inline auto size() {
				std::shared_lock lock(mutex);
				return std::queue<T, C>::size();
			}

			inline C steal() {
				std::unique_lock lock(mutex);
				return std::move(this->c);
			}

			inline const C & peek() {
				return this->c;
			}

			inline auto lock_shared() {
				return std::shared_lock(mutex);
			}

			inline auto lock_unique() {
				return std::unique_lock(mutex);
			}
	};
}
