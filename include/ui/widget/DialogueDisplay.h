#pragma once

#include "ui/widget/Box.h"

namespace Game3 {
	class Label;

	class DialogueDisplay: public Box {
		public:
			DialogueDisplay(UIContext &, float selfScale);

			void init() final;

		private:
			std::shared_ptr<Label> mainText;
			std::shared_ptr<Box> optionBox;
	};
}
