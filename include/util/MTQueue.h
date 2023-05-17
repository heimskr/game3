#pragma once

#include <mutex>
#include <queue>
#include <shared_mutex>

namespace Game3 {
	/** A threadsafe but slow queue. */
	template <typename T, typename C = std::deque<T>>
	class MTQueue: private std::queue<T, C> {
		private:
			std::shared_mutex mutex;

		public:
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
