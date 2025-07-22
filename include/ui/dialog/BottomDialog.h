#pragma once

#include "ui/dialog/Dialog.h"

namespace Game3 {
	class Hotbar;

	class BottomDialog: public Dialog {
		public:
			BottomDialog(UIContext &, float selfScale);

			void init() final;
			void render(const RendererContext &) final;
			Rectangle getPosition() const final;

			std::shared_ptr<Hotbar> getHotbar() const;

		private:
			std::shared_ptr<Hotbar> hotbar;

			bool shouldShowHotbar() const;
	};
}
