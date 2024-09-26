#pragma once

#include "ui/gl/dialog/Dialog.h"

#include <memory>

namespace Game3 {
	class ConnectionDialog: public Dialog {
		public:
			ConnectionDialog(UIContext &);

			void init() final;
			void render(const RendererContext &) final;
			Rectangle getPosition() const final;
	};
}
