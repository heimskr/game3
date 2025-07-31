#pragma once

#include <functional>
#include <optional>

namespace Game3 {
	template <typename T>
	class Noticer {
		public:
			Noticer(std::function<void(const T *old, const T &updated)> function = {}):
				function(std::move(function)) {}

			Noticer(T value, std::function<void(const T *old, const T &updated)> function = {}):
				oldValue(std::move(value)),
				function(std::move(function)) {}

			bool update(const T &value) {
				if (oldValue == value) {
					return false;
				}

				if (function) {
					function(getValue(), value);
				}

				oldValue = value;
				return true;
			}

			void setFunction(std::function<void(const T *old, const T &updated)> new_function) {
				function = std::move(new_function);
			}

			const T * getValue() const {
				return oldValue? &*oldValue : nullptr;
			}

		private:
			std::optional<T> oldValue;
			std::function<void(const T *old, const T &updated)> function;
	};
}
