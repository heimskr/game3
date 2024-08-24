#pragma once

#include "types/Types.h"
#include "ui/gl/Dialog.h"
#include "ui/gl/ItemSlotWidget.h"

#include <memory>
#include <vector>

namespace Game3 {
	class Player;

	class OmniDialog: public Dialog {
		private:
			std::shared_ptr<Player> player;
			std::vector<std::shared_ptr<ItemSlotWidget>> slotWidgets;
			Slot previousActive = -1;
			Rectangle innerRectangle;

		public:
			OmniDialog(UIContext &, std::shared_ptr<Player>);

			void render(RendererContext &) final;
			Rectangle getPosition() const final;
			bool click(int x, int y) final;
			bool dragStart(int x, int y) final;
			bool dragEnd(int x, int y) final;
	};
}
