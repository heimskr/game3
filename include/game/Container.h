#pragma once

namespace Game3 {
	struct Container {
		virtual ~Container() = default;

		template <typename T>
		bool is() const {
			return dynamic_cast<const T *>(this) != nullptr;
		}
	};
}
