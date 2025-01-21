#pragma once

#include <deque>
#include <mutex>
#include <optional>
#include <shared_mutex>

namespace Game3 {
	/** A threadsafe but slow queue. */
	template <typename T>
	class MTQueue: private std::deque<T> {
		private:
			mutable std::shared_mutex mutex;

		public:
			MTQueue() = default;
			using std::deque<T>::deque;

			inline T & front() {
				std::shared_lock lock(mutex);
				return std::deque<T>::front();
			}

			inline const T & front() const {
				std::shared_lock lock(mutex);
				return std::deque<T>::front();
			}

			inline T & back() {
				std::shared_lock lock(mutex);
				return std::deque<T>::back();
			}

			inline const T & back() const {
				std::shared_lock lock(mutex);
				return std::deque<T>::back();
			}

			inline void pop() {
				std::unique_lock lock(mutex);
				std::deque<T>::pop_front();
			}

			inline T take() {
				std::unique_lock lock(mutex);
				T out = std::move(std::deque<T>::front());
				std::deque<T>::pop_front();
				return out;
			}

			inline std::optional<T> tryTake() {
				std::unique_lock lock(mutex);
				if (std::deque<T>::empty()) {
					return std::nullopt;
				}
				auto out = std::make_optional(std::move(std::deque<T>::front()));
				std::deque<T>::pop_front();
				return out;
			}

			inline void push(T &&value) {
				std::unique_lock lock(mutex);
				std::deque<T>::push_back(std::move(value));
			}

			inline void push(const T &value) {
				std::unique_lock lock(mutex);
				std::deque<T>::push_back(value);
			}

			inline bool empty() const {
				std::shared_lock lock(mutex);
				return std::deque<T>::empty();
			}

			inline void clear() {
				std::unique_lock lock(mutex);
				std::deque<T>::clear();
			}

			template <typename... Args>
			inline auto emplace(Args &&...args) {
				std::unique_lock lock(mutex);
				return std::deque<T>::emplace_back(std::forward<Args>(args)...);
			}

			inline auto size() const {
				std::shared_lock lock(mutex);
				return std::deque<T>::size();
			}

			inline std::deque<T> steal() {
				std::unique_lock lock(mutex);
				return std::move(static_cast<std::deque<T> &>(*this));
			}

			inline auto sharedLock() {
				return std::shared_lock(mutex);
			}

			inline auto uniqueLock() {
				return std::unique_lock(mutex);
			}

			inline std::deque<T> & get() {
				return static_cast<std::deque<T> &>(*this);
			}

			inline const std::deque<T> & get() const {
				return static_cast<const std::deque<T> &>(*this);
			}
	};
}
