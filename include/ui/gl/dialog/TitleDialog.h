#pragma once

#include "tools/Updater.h"
#include "ui/gl/dialog/Dialog.h"

namespace Game3 {
	class Aligner;
	class Box;
	class IconButton;

	class TitleDialog final: public Dialog {
		public:
			TitleDialog(UIContext &, float selfScale);

			void init() final;
			void render(const RendererContext &) final;
			Rectangle getPosition() const final;

		private:
			Updater updater;
			std::shared_ptr<Aligner> aligner;
			std::shared_ptr<Box> hbox;
			std::shared_ptr<IconButton> updateButton;
	};
}
