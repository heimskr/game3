#pragma once

#include "data/Identifier.h"
#include "registry/Registerable.h"

namespace Game3 {
	class RegisterableIdentifier: public NamedRegisterable {
		public:
			RegisterableIdentifier(Identifier name, Identifier value):
				NamedRegisterable(std::move(name)), value(std::move(value)) {}

			Identifier & get() {
				return value;
			}

			const Identifier & get() const {
				return value;
			}

			operator Identifier &() {
				return value;
			}

			operator const Identifier &() const {
				return value;
			}

		private:
			Identifier value;
	};
}
