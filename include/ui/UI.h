#pragma once

#include "ui/dialog/Dialog.h"

namespace Game3 {
	class Window;

	class UI: public Dialog {
		public:
			using Dialog::Dialog;

			virtual Identifier getID() const = 0;

			using Dialog::init;
			virtual void init(Window &) = 0;

			/** Called if nothing else handled the key press. */
			virtual bool keyPressed(uint32_t key, Modifiers modifiers, bool is_repeat);

			/** Called if nothing else handled the char press. */
			virtual bool charPressed(uint32_t codepoint, Modifiers modifiers);
	};
}
