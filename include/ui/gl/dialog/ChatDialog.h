#pragma once

#include "types/UString.h"
#include "ui/gl/dialog/Dialog.h"

namespace Game3 {
	class Box;
	class Label;
	class Scroller;
	class TextInput;

	class ChatDialog: public Dialog {
		public:
			ChatDialog(UIContext &);

			void init() override;
			void render(const RendererContext &) override;
			Rectangle getPosition() const override;
			bool click(int button, int x, int y) override;
			bool scroll(float x_delta, float y_delta, int x, int y) override;
			bool keyPressed(uint32_t key, Modifiers, bool is_repeat) override;
			void onFocus() override;
			void onBlur() override;

			void addMessage(UString);
			void toggle(bool affect_focus);
			void setHidden(bool);

		private:
			std::shared_ptr<Scroller> scroller;
			std::shared_ptr<Box> messageBox;
			std::shared_ptr<Box> vbox;
			std::shared_ptr<TextInput> messageInput;
			std::shared_ptr<Label> toggler;
			bool isHidden = false;
	};
}
