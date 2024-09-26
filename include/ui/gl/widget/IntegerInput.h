#pragma once

#include "ui/gl/widget/TextInput.h"

namespace Game3 {
	class IntegerInput: public TextInput {
		public:
			template <typename... Args>
			IntegerInput(Args &&...args):
				TextInput(std::forward<Args>(args)...) {
					characterFilter = [](uint32_t character, const UString::iterator &) {
						return std::isdigit(static_cast<int>(character));
					};
				}
	};
}
