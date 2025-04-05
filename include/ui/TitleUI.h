#pragma once

#include "tools/Updater.h"
#include "ui/UI.h"

namespace Game3 {
	class Aligner;
	class Box;
	class IconButton;

	class TitleUI final: public UI {
		public:
			using UI::UI;

			void init(Window &) final;
			void render(const RendererContext &) final;
			Rectangle getPosition() const final;

		private:
			UpdaterPtr updater;
			std::shared_ptr<Aligner> aligner;
			std::shared_ptr<Box> hbox;
			std::shared_ptr<IconButton> updateButton;
	};
}
