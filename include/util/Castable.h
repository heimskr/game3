#pragma once

#include <memory>

namespace Game3 {
	template <typename T>
	class Castable: public std::enable_shared_from_this<T> {
		public:
			template <typename S>
			std::shared_ptr<S> cast() {
				return std::dynamic_pointer_cast<S>(shared_from_this());
			}

			template <typename S>
			std::shared_ptr<const S> cast() const {
				return std::dynamic_pointer_cast<const S>(shared_from_this());
			}
	};
}
