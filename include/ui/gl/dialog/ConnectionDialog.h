#pragma once

#include "ui/gl/dialog/Dialog.h"

#include <memory>

namespace Game3 {
	class TextInput;
	class IntegerInput;

	class ConnectionDialog: public Dialog {
		public:
			ConnectionDialog(UIContext &);

			void init() final;
			void render(const RendererContext &) final;
			Rectangle getPosition() const final;

		private:
			std::shared_ptr<TextInput> hostInput;
			std::shared_ptr<IntegerInput> portInput;
			void submit();
	};
}
