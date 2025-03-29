#pragma once

#include "ui/gl/dialog/SizeCachingDialog.h"

#include <memory>

namespace Game3 {
	class TextInput;
	class IntegerInput;

	class ConnectionDialog: public Dialog {
		public:
			ConnectionDialog(UIContext &, float selfScale);

			void init() final;
			void render(const RendererContext &) final;
			Rectangle getPosition() const final;

		private:
			std::shared_ptr<TextInput> hostInput;
			std::shared_ptr<IntegerInput> portInput;
			void submit();
			void playLocally();
	};
}
