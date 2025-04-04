#pragma once

#include "ui/UI.h"

namespace Game3 {
	class TitleUI final: public UI {
		public:
			void render(Window &) final;
	};
}
