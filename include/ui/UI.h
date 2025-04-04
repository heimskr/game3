#pragma once

#include "ui/gl/dialog/Dialog.h"

namespace Game3 {
	class Window;

	class UI: public Dialog {
		public:
			using Dialog::Dialog;

			using Dialog::init;
			virtual void init(Window &) = 0;
	};
}
