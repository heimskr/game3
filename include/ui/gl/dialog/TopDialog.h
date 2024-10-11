#pragma once

#include "ui/gl/dialog/Dialog.h"

namespace Game3 {
	class Box;
	class Icon;

	class TopDialog: public Dialog {
		public:
			TopDialog(UIContext &);

			void init() final;
			void render(const RendererContext &) final;
			Rectangle getPosition() const final;
			void onBlur() final;

		private:
			std::shared_ptr<Box> hbox;
			std::shared_ptr<Icon> abscond;
	};
}
