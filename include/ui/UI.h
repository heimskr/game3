#pragma once

#include "ui/gl/dialog/Dialog.h"

namespace Game3 {
	class Window;

	class UI: public Dialog {
		public:
			using Dialog::Dialog;

			virtual Identifier getID() const = 0;

			using Dialog::init;
			virtual void init(Window &) = 0;
	};
}
