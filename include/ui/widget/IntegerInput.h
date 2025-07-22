#pragma once

#include "ui/widget/TextInput.h"
#include "util/Util.h"

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

			template <std::integral T>
			T getValue(std::optional<T> if_empty = {}) const {
				if (if_empty && getText().empty()) {
					return *if_empty;
				}

				return parseNumber<T>(getText().raw());
			}

			template <std::integral T>
			std::optional<T> tryValue() const {
				if (getText().empty()) {
					return std::nullopt;
				}

				try {
					return parseNumber<T>(getText().raw());
				} catch (const std::invalid_argument &) {
					return std::nullopt;
				}
			}
	};
}
