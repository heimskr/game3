#pragma once

#include <cassert>
#include <functional>

namespace Game3 {
	class Defer {
		public:
			Defer(std::function<void()> function = {}):
				function(std::move(function)) {}

			~Defer() {
				if (function) {
					function();
				}
			}

			template <std::invocable T>
			Defer & operator=(T new_function) {
				function = std::move(new_function);
				return *this;
			}

		private:
			std::function<void()> function;
	};
}
