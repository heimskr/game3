#pragma once

#include <cstdint>

namespace Game3 {
	class Container {
		public:
			uint64_t updateCounter = 0;

			virtual ~Container() = default;

			template <typename T>
			bool is() const {
				return dynamic_cast<const T *>(this) != nullptr;
			}

			virtual void increaseUpdateCounter() {
				++updateCounter;
			}
	};
}
