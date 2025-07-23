#pragma once

#include "ui/dialog/Dialog.h"

namespace Game3 {
	class DialogueDisplay;
	class Scroller;

	class DialogueDialog: public Dialog {
		public:
			DialogueDialog(UIContext &, float selfScale);

			void init() final;
			void render(const RendererContext &) final;
			Rectangle getPosition() const final;
			bool hidesHotbar() const final;
			bool keyPressed(uint32_t key, Modifiers, bool is_repeat) final;

		private:
			std::shared_ptr<Scroller> dialogueScroller;
			std::shared_ptr<DialogueDisplay> dialogueDisplay;
			TexturePtr faceSprite;
	};
}
