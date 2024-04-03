#pragma once

#include <cassert>
#include <functional>

namespace Game3 {
	class Defer {
		public:
			Defer(std::function<void()> function_): function(std::move(function_)) {
				assert(function);
			}

			~Defer() {
				function();
			}

		private:
			std::function<void()> function;
	};
}
