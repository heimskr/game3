#pragma once

#include "ui/gl/Dialog.h"

namespace Game3 {
	class InventoryDialog: public Dialog {
		public:
			InventoryDialog() = default;

			void render(UIContext &, RendererContext &) final;
	};
}
