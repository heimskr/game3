#pragma once

#include "ui/gl/Dialog.h"

namespace Game3 {
	class Player;

	class InventoryDialog: public Dialog {
		private:
			std::shared_ptr<Player> player;

		public:
			InventoryDialog(std::shared_ptr<Player>);

			void render(UIContext &, RendererContext &) final;
	};
}
